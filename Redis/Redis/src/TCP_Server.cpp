#include "TCP_Server.h"

// ==============================
//	Marcros, 
//	static member variables
//	& other junk
// ==============================

#define CMD_IS(word, cmd) (_stricmp((word).c_str(), (cmd)) == 0)
std::map<std::string, std::string> TCP_Server::g_map;

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
	//HandleConnections();
	HandleConnections2();
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

	SetNonBlockingFD(this->fd);
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

void TCP_Server::HandleConnections2()
{
	SetNonBlockingFD(this->fd);

	while (true)
	{
		fd_set readfds, writefds, exceptfds;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		FD_SET(this->fd, &readfds);
		FD_SET(this->fd, &exceptfds);

		SOCKET maxfd = this->fd;
		for (const auto& pConn : this->fd2conn)
		{
			if (!pConn)
			{
				continue;
			}
			if (pConn->state == STATE_REQUEST)
			{
				FD_SET(pConn->fd, &readfds);
			}
			else if (pConn->state == STATE_RESPONSE)
			{
				FD_SET(pConn->fd, &writefds);
			}
			FD_SET(pConn->fd, &exceptfds);
			if (pConn->fd > maxfd)
			{
				maxfd = pConn->fd;
			}
		}

		timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int rv = select(static_cast<int>(maxfd + 1), &readfds, &writefds, &exceptfds, &timeout);
		if (rv == SOCKET_ERROR)
		{
			Die("select()");
		}

		if (rv == 0)
		{
			continue;
		}

		if (FD_ISSET(this->fd, &readfds))
		{
			AcceptNewConnection(fd2conn, this->fd);
		}

		for (auto it = this->fd2conn.begin(); it != this->fd2conn.end(); )
		{
			auto& pConn = *it;
			if (pConn)
			{
				if (FD_ISSET(pConn->fd, &exceptfds))
				{
					ConnectionIO(pConn);
				}
				else if (FD_ISSET(pConn->fd, &readfds))
				{
					ConnectionIO(pConn);
				}
				else if (FD_ISSET(pConn->fd, &writefds))
				{
					ConnectionIO(pConn);
				}

				if (pConn->state == STATE_END)
				{
					closesocket(pConn->fd);
					delete pConn;
					it = this->fd2conn.erase(it);
					continue;
				}
			}
			++it;
		}
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

void TCP_Server::StateRequest(Conn* conn)
{
	while (TryFillBuffer(conn)) {}
}

void TCP_Server::StateResponse(Conn* conn)
{
	while (TryFlushBuffer(conn)) {}
}

bool TCP_Server::TryOneRequest(Conn* conn)
{
	// Try to parse a request from the buffer
	if (conn->rbuff_size < 4)
	{
		// not enough data in the buffer,
		// will retry in the next iteration
		return false;
	}
	uint32_t len = 0;

	memcpy(&len, &conn->rbuff[0], 4);
	if (len > kMaxMsg)
	{
		Msg("Message too long");
		conn->state = STATE_END;
		return false;
	}

	if (4 + len > conn->rbuff_size)
	{
		// not enough data in the buffer, 
		// will retry in the next iteration
		return false;
	}

	// got one request, generate the response.
	uint32_t rescode = 0;
	uint32_t wlen = 0;
	int32_t err = DoRequest(&conn->rbuff[4], len, &rescode, &conn->wbuff[4 + 4], &wlen);

	if (err) 
	{
		conn->state = STATE_END;
		return false;
	}
	wlen += 4;
	memcpy(&conn->wbuff[0], &wlen, 4);
	memcpy(&conn->wbuff[4], &rescode, 4);
	conn->wbuff_size = 4 + wlen;

	size_t remain = conn->rbuff_size - 4 - len;
	if (remain) 
	{
		memmove(conn->rbuff, &conn->rbuff[4 + len], remain);
	}
	conn->rbuff_size = remain;

	// change state
	conn->state = STATE_RESPONSE;
	StateResponse(conn);

	// continue the outer loop if the request was fully processed
	return (conn->state == STATE_REQUEST);
}

bool TCP_Server::TryFillBuffer(Conn* conn)
{
	// Try to fill the buffer

	assert(conn->rbuff_size < sizeof(conn->rbuff));
	SSIZE_T rv = 0;

	do
	{
		size_t cap = sizeof(conn->rbuff) - conn->rbuff_size;
		rv = recv(conn->fd, (char*)&conn->rbuff[conn->rbuff_size], cap, 0);

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
	conn->rbuff_size += (size_t)rv;

	assert(conn->rbuff_size <= sizeof(conn->rbuff));

	while (TryOneRequest(conn)) {}

	return (conn->state == STATE_REQUEST);
}

bool TCP_Server::TryFlushBuffer(Conn* conn)
{
	SSIZE_T rv = 0;

	do
	{
		size_t remaining = conn->wbuff_size - conn->wbuff_sent;
		rv = send(conn->fd, (char*)(&conn->wbuff[conn->wbuff_sent]), remaining, 0);
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

	conn->wbuff_sent += (size_t)rv;
	
	assert(conn->wbuff_sent <= conn->wbuff_size);

	if (conn->wbuff_sent == conn->wbuff_size)
	{
		conn->wbuff_sent = 0;
		conn->wbuff_size = 0;
		conn->state = STATE_REQUEST;
		return false;
	}

	return true;
}

void TCP_Server::ConnectionIO(Conn* conn)
{
	if (conn->state == STATE_REQUEST)
	{
		StateRequest(conn);
	}

	else if (conn->state == STATE_RESPONSE)
	{
		StateResponse(conn);
	}
	
	else
	{
		assert(0);	// not expected
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
	
	// set the new connection fd to nonblocking mode
	SetNonBlockingFD(connfd);

	// creating the struct Conn
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

int32_t TCP_Server::WriteAll(SOCKET fd, const char* buff, size_t n)
{
	while (n > 0) 
	{
		size_t rv = send(fd, buff, n, 0);
		if (rv <= 0) {
			std::cout << "WriteAll(): " << WSAGetLastError() << std::endl;
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
	char rbuff[4 + kMaxMsg + 1];
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
	if (len > kMaxMsg)
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

	return WriteAll(connfd, wbuff, 4 + len);
}

int32_t TCP_Server::ParseRequest(const uint8_t* data, size_t len, std::vector<std::string>& out)
{
	if (len < 4) 
	{
		return -1;
	}

	uint32_t n = 0;
	memcpy(&n, &data[0], 4);
	if (n > kMaxArgs) 
	{
		return -1;
	}

	size_t pos = 4;
	while (n--) 
	{
		if (pos + 4 > len) 
		{
			return -1;
		}
		uint32_t sz = 0;
		memcpy(&sz, &data[pos], 4);
		if (pos + 4 + sz > len) 
		{
			return -1;
		}
		out.push_back(std::string((char*)&data[pos + 4], sz));
		pos += 4 + sz;
	}

	if (pos != len) 
	{
		return -1;  // trailing garbage
	}

	return 0;
}

uint32_t TCP_Server::DoGet(const std::vector<std::string>& cmd, uint8_t* res, uint32_t* reslen)
{
	if (!g_map.count(cmd[1])) 
	{
		return RESPONSE_NOT_FOUND;
	}

	std::string& val = g_map[cmd[1]];
	assert(val.size() <= kMaxMsg);
	memcpy(res, val.data(), val.size());
	*reslen = (uint32_t)val.size();

	return RESPONSE_OK;
}

uint32_t TCP_Server::DoSet(const std::vector<std::string>& cmd, uint8_t* res, uint32_t* reslen)
{
	(void)res;
	(void)reslen;
	g_map[cmd[1]] = cmd[2];
	return RESPONSE_OK;
}

uint32_t TCP_Server::DoDel(const std::vector<std::string>& cmd, uint8_t* res, uint32_t* reslen)
{
	(void)res;
	(void)reslen;
	g_map.erase(cmd[1]);
	return RESPONSE_OK;
}

int32_t TCP_Server::DoRequest(const uint8_t* req, uint32_t reqlen, uint32_t* rescode, uint8_t* res, uint32_t* reslen)
{
	std::vector<std::string> cmd;
	if (0 != ParseRequest(req, reqlen, cmd)) 
	{
		Msg("bad request");
		return -1;
	}

	if (cmd.size() == 2 && CMD_IS(cmd[0], "get")) 
	{
		*rescode = DoGet(cmd, res, reslen);
	}
	else if (cmd.size() == 3 && CMD_IS(cmd[0], "set"))
	{
		*rescode = DoSet(cmd, res, reslen);
	}
	else if (cmd.size() == 2 && CMD_IS(cmd[0], "del"))
	{
		*rescode = DoDel(cmd, res, reslen);
	}
	else 
	{
		// cmd is not recognized
		*rescode = RESPONSE_ERROR;
		const char* msg = "Unknown cmd";
		*reslen = strlen(msg);
		strcpy_s((char*)res, *reslen, msg);
		return 0;
	}

	return 0;
}