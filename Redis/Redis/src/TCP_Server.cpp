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

int32_t TCP_Server::OneRequest(SOCKET connfd)
{	
	// 4 bytes for header, kMaxSize for actual message, 1 byte for null terminator
	char rbuff[4 + kMaxSize + 1];

	errno = 0;
	int32_t err = ReadFull(connfd, rbuff, sizeof(rbuff));
	if (err)
	{
		if (errno == 0)
		{
			Msg("EOF");
		}
		else
		{
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
	std::cout << "Client says: " << rbuff[4] << std::endl;

	const char reply[] = "World";
	char wbuff[4 + sizeof(reply)];
	len = (uint32_t)strlen(reply);
	memcpy(wbuff, &len, 4);
	memcpy(&wbuff[4], reply, len);

	return WriteFull(connfd, wbuff, 4 + len);
}

int32_t TCP_Server::ReadFull(SOCKET fd, char* buff, size_t n)
{
	while (n > 0)
	{
		size_t rv = recv(fd, buff, n, 0);
		if (rv <= 0)
		{
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
			return -1;
		}

		assert(static_cast<size_t>(rv) <= n);

		n -= static_cast<size_t>(rv);
		buff += rv;
	}
	return 0;
}
