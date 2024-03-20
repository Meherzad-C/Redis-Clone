#include "TCP_Server.h"

// ==============================
//	Constructors and Destructors
// ==============================

TCP_Server::TCP_Server(int port) : port(port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		Die("WSAStartup() failed");
	}

	SetupSocket();
	BindSocket();
	ListenForConnections();
}

TCP_Server::~TCP_Server()
{
	closesocket(this->fd);
	WSACleanup();
}

// ==============================
//	Public Member Functions
// ==============================

void TCP_Server::Start()
{
	HandleConnections();
}

// ==============================
//	Private Member Functions
// ==============================

void TCP_Server::SetupSocket()
{
	this->fd = socket(AF_INET, SOCK_STREAM, 0);

	if (this->fd < 0)
	{
		Die("socket() failed");
	}

	// Enable reuse of the address in case of TIME_WAIT state, to ensure 
	// that any remaining packets in the network are handled properly.
	int val = 1;
	setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(& val), sizeof(val));
}

void TCP_Server::BindSocket()
{
	struct sockaddr_in addr = {};

	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(this->port);
	addr.sin_addr.s_addr = ntohl(INADDR_ANY);

	int bindVal = bind(this->fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

	if (bindVal < 0)
	{
		Die("bind() failed");
	}
}

void TCP_Server::ListenForConnections()
{
	int rv = listen(this->fd, SOMAXCONN);

	if (rv < 0)
	{
		Die("listen() failed");
	}
}

void TCP_Server::HandleConnections()
{
	while (true)
	{
		struct sockaddr_in client_addr = {};
		int addrlen = sizeof(client_addr);

		SOCKET connfd = accept(this->fd, (struct sockaddr*)&client_addr, &addrlen);

		if (connfd == INVALID_SOCKET)
		{
			continue;
		}
		while (true)
		{
			int32_t err = OneRequest(connfd);
			if (err)
			{
				break;
			}
		}
		/*DoSomething(connfd);*/
		closesocket(connfd);
	}

}

void TCP_Server::SetNonBlockingFD(SOCKET fd)
{
	u_long mode = 1;					// 0 for blocking, 1 for non-blocking
	int result = ioctlsocket(fd, FIONBIO, &mode);

	if (result == SOCKET_ERROR)
	{
		std::cerr << "ioctlsocket() error, " << WSAGetLastError() << std::endl;
	}
}

void TCP_Server::DoSomething(SOCKET connfd)
{
	char rbuf[64] = {};
	size_t n = recv(connfd, rbuf, sizeof(rbuf) - 1, 0);
	if (n < 0) {
		Msg("recv() error");
		return;
	}
	printf("client says: %s\n", rbuf);

	const char wbuf[] = "world";
	send(connfd, wbuf, sizeof(wbuf), 0);
}

void TCP_Server::ConnPut(std::vector<Conn*>& fd2conn, Conn* conn)
{
	if (fd2conn.size() <= static_cast<size_t>(conn->fd))
	{
		fd2conn.resize(conn->fd + 1);
	}

	fd2conn[conn->fd] = conn;
}

bool TCP_Server::TryOneRequest(Conn* conn)
{
	// Try to parse a request from the buffer
	if (conn->rbuff_size < 4)
	{
		return false;
	}
	uint32_t len;

	memcpy(&len, &conn->rbuff[0], 4);
	if (len > kMaxSize)
	{
		Msg("Message too long");
		conn->state = STATE_END;
		return false;
	}

	if (4 + len > conn->rbuff_size)
	{
		// not enough data in the buffer. Will retry in the next iteration
		return false;
	}

	// got one request, do something with it
	printf("client says: %.*s\n", len, &conn->rbuff[4]);

	// generating echoing response
	memcpy(&conn->wbuff[0], &len, 4);
	memcpy(&conn->wbuff[4], &conn->rbuff[4], len);
	conn->wbuff_size = 4 + len;

	size_t remain = conn->rbuff_size - 4 - len;
	if (remain) {
		memmove(conn->rbuff, &conn->rbuff[4 + len], remain);
	}
	conn->rbuff_size = remain;

	// change state
	conn->state = STATE_RESPONSE;
	StateResponse(conn);

	// continue the outer loop if the request was fully processed
	return (conn->state == STATE_REQUEST);
}

void TCP_Server::StateRequest(Conn* conn)
{
	while (TryFillBuffer(conn)) {}
}

void TCP_Server::StateResponse(Conn* conn)
{
	while (TryFlushBuffer(conn)) {}
}

bool TCP_Server::TryFillBuffer(Conn* conn)
{
	// Try to fill the buffer
	assert(conn->rbuff_size < sizeof(conn->rbuff));
	size_t rv = 0;

	do
	{
		size_t cap = sizeof(conn->rbuff) - conn->rbuff_size;
		rv = recv(conn->fd, reinterpret_cast<char*>(&conn->rbuff[conn->rbuff_size]), cap, 0);

	} while (rv < 0 && WSAGetLastError() == WSAEINTR);

	if (rv < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
	{
		return false;
	}

	if (rv < 0)
	{
		Msg("recv() error");
		conn->state = STATE_END;
		return false;
	}
	if (rv == 0)
	{
		if (conn->rbuff_size > 0)
		{
			Msg("Unexpected EOF");
		}
		else
		{
			Msg("EOF");
		}
		conn->state = STATE_END;
		return false;
	}
	conn->rbuff_size += rv;
	assert(conn->rbuff_size <= sizeof(conn->rbuff));

	while (TryOneRequest(conn)) {}

	return (conn->state == STATE_REQUEST);
}

bool TCP_Server::TryFlushBuffer(Conn* conn)
{
	size_t rv = 0;

	do
	{
		size_t remaining = conn->wbuff_size - conn->wbuff_sent;
		rv = send(conn->fd, reinterpret_cast<const char*>(&conn->wbuff[conn->wbuff_sent]), remaining, 0);
	} while (rv < 0 && WSAGetLastError() == WSAEINTR);

	if (rv < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
	{
		return false;
	}

	if (rv < 0)
	{
		int error = WSAGetLastError();
		fprintf(stderr, "send() error %d\n", error);
		conn->state = STATE_END;
		return false;
	}

	conn->wbuff_size += rv;
	assert(conn->wbuff_sent <= conn->wbuff_size);

	if (conn->rbuff_size == conn->wbuff_size)
	{
		conn->wbuff_sent = 0;
		conn->wbuff_size = 0;
		conn->state = STATE_REQUEST;
		return false;
	}

	return true;
}

void TCP_Server::Connection_IO(Conn* conn)
{
	if (conn->state == STATE_REQUEST)
	{
		StateRequest(conn);
	}

	if (conn->state == STATE_RESPONSE)
	{
		StateResponse(conn);
	}
	
	else
	{
		// not expected
		assert(0);
	}
}

int32_t TCP_Server::AcceptNewConnection(std::vector<Conn*>& fd2conn, int fd)
{
	struct sockaddr_in clientAddr = {};
	int clientAddrLen = sizeof(clientAddr);
	SOCKET connfd = accept(fd, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);

	if (connfd == INVALID_SOCKET)
	{
		Msg("accept() error");
		return -1;
	}
	
	SetNonBlockingFD(connfd);

	Conn* conn = (Conn*)malloc(sizeof(Conn));

	if (!conn)
	{
		closesocket(connfd);
		return -1;
	}

	conn->fd = connfd;
	conn->state = STATE_REQUEST;
	conn->rbuff_size = 0;
	conn->wbuff_size = 0;
	conn->wbuff_sent = 0;

	ConnPut(fd2conn, conn);

	return 0;
}

void TCP_Server::Msg(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
}

void TCP_Server::Die(const char* msg)
{
	int err = WSAGetLastError();
	fprintf(stderr, "[%d] %s\n", err, msg);
	exit(EXIT_FAILURE);
}

int32_t TCP_Server::ReadFull(SOCKET fd, char* buff, size_t n)
{
	while (n > 0)
	{
		size_t rv = recv(fd, buff, n, 0);
		if (rv <= 0)
		{
			std::cout << "ReadFull(): " << WSAGetLastError() << std::endl;
			return -1;
		}
		
		assert(static_cast<size_t>(rv) <= n);

		n -= static_cast<size_t>(rv);
		buff += rv;
	}

	return 0;
}

int32_t TCP_Server::WriteFull(SOCKET fd, const char* buff, size_t n)
{
	while (n > 0) 
	{
		size_t rv = send(fd, buff, n, 0);
		if (rv <= 0) {
			std::cout << "WriteFull(): " << WSAGetLastError() << std::endl;
			return -1;
		}

		assert(static_cast<size_t>(rv) <= n);

		n -= static_cast<size_t>(rv);
		buff += rv;
	}
	return 0;
}

int32_t TCP_Server::OneRequest(SOCKET connfd)
{
	// 4 bytes for header, kMaxSize for actual message, 1 byte for null terminator
	char rbuff[4 + kMaxSize + 1];
	//memcpy(rbuff, 0, sizeof(rbuff));
	int getError = WSAGetLastError();
	int32_t err = ReadFull(connfd, rbuff, 4);
	if (err)
	{
		if (getError == 0)
		{
			Msg("EOF");
		}
		else
		{
			std::cout << "Error at OneRequest(): " << WSAGetLastError() << std::endl;
			Msg("read() error");
		}
		return err;
	}

	uint32_t len = 0;
	memcpy(&len, rbuff, 4);
	if (len > kMaxSize)
	{
		Msg("too long");
		return -1;
	}

	// request actual message body
	err = ReadFull(connfd, &rbuff[4], len);
	if (err)
	{
		Msg("read() error");
	}

	rbuff[4 + len] = '\0';
	std::cout << "Client says: " << &rbuff[4] << std::endl;
	printf("client says: %s\n", &rbuff[4]);

	const char reply[] = "World";
	char wbuff[4 + sizeof(reply)];
	len = (uint32_t)strlen(reply);
	memcpy(wbuff, &len, 4);
	memcpy(&wbuff[4], reply, len);

	return WriteFull(connfd, wbuff, 4 + len);
}