#include "TCP_Client.h"

// ==============================
//	Constructors and Destructors
// ==============================

TCP_Client::TCP_Client()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Die("WSAStartup() failed");
	}

	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd < 0)
	{
		Die("Socket failed");
	}
}

TCP_Client::~TCP_Client()
{
	closesocket(this->fd);
	WSACleanup();
}

// ==============================
//	Public Member Functions
// ==============================

bool TCP_Client::ConnectToServer(const char* serverAddress, int port)
{
	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (inet_pton(AF_INET, serverAddress, &addr.sin_addr) <= 0)
	{
		Die("inet_pton() failed");
		return false;
	}

	int rv = connect(this->fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (rv < SOCKET_ERROR)
	{
		Die("connect() failed");
		return false;
	}

	return true;
}

void TCP_Client::Send_Message(const char* message)
{
	size_t msgLen = strlen(message);
	send(this->fd, message, msgLen, 0);
}

void TCP_Client::Receive_Message(char* buffer, size_t bufferSize)
{
	// buffer -1 is used directly in the recv() so the received data is null-terminated to ensure proper string handling.
	size_t n = recv(this->fd, buffer, bufferSize - 1, 0);

	if (n == SOCKET_ERROR)
	{
		Die("recv() failed");
	}

	buffer[n] = '\0';
}

int32_t TCP_Client::QueryServer(SOCKET fd, const char* text)
{
	uint32_t len = static_cast<uint32_t>(strlen(text));
	if (len > kMaxSize)
	{
		return -1;
	}

	char wbuff[4 + kMaxSize];
	//memcpy(wbuff, 0, sizeof(wbuff));
	memcpy(wbuff, &len, 4);
	memcpy(&wbuff[4], text, len);

	if (int32_t err = WriteAll(fd, wbuff, 4 + len))
	{
		return err;
	}

	// 4 header bytes to store message length and kMaxSize to store actual message 
	char rbuff[4 + kMaxSize + 1];

	int getError = WSAGetLastError();

	int32_t err = ReadFull(fd, rbuff, 4);
	if (err)
	{
		if (getError == 0)
		{
			Msg("EOF");
		}
		else
		{
			std::cout << "ReadFull(): " << WSAGetLastError() << std::endl;
			Msg("read() error");
		}
	}

	memcpy(&len, rbuff, 4);
	if (len > kMaxSize)
	{
		Msg("Message too long!");
	}

	err = ReadFull(fd, &rbuff[4], len);
	if (err)
	{
		Msg("read() error");
		return err;
	}

	rbuff[4 + len] = '\0';

	std::cout << "Server says: " << &rbuff[4] << std::endl;
	printf("server says: %s\n", &rbuff[4]);

	return 0;
}

// ==============================
//	Private Member Functions
// ==============================

void TCP_Client::Msg(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
}

void TCP_Client::Die(const char* msg)
{
	int errorMsg = WSAGetLastError();
	std::cerr << "[" << errorMsg << "] " << msg << std::endl;

	exit(EXIT_FAILURE);
}

int32_t TCP_Client::ReadFull(SOCKET fd, char* buff, size_t n)
{
	while (n > 0) {
		size_t rv = recv(fd, buff, n, 0);
		if (rv <= 0) {
			return -1;
		}
		assert(static_cast<size_t>(rv) <= n);
		n -= static_cast<size_t>(rv);
		buff += rv;
	}
	return 0;
}

int32_t TCP_Client::WriteAll(SOCKET fd, const char* buff, size_t n)
{
	while (n > 0)
	{
		size_t rv = send(fd, buff, n, 0);
		if (rv <= 0)
		{
			std::cout << "WriteFull(): " << WSAGetLastError() << std::endl;
			return -1;  // error
		}
		assert(static_cast<size_t>(rv) <= n);
		n -= (size_t)rv;
		buff += rv;
	}
	return 0;
}

SOCKET TCP_Client::GetSocket() const
{
	return this->fd;
}

int32_t TCP_Client::SendRequest(SOCKET fd, const char* text)
{
	uint32_t len = static_cast<uint32_t>(strlen(text));
	
	// do not overflow
	if (len > kMaxSize)
	{
		Msg("Error SendingRequest()");
		return -1;
	}

	char wbuff[4 + kMaxSize];
	memcpy(wbuff, &len, 4);
	memcpy(&wbuff[4], text, len);

	return WriteAll(fd, wbuff, 4 + len);
}

int32_t TCP_Client::ReadRequest(SOCKET fd)
{
	// 4=len, kMaxSize=sizeof Message, 1=string delimiter
	char rbuff[4 + kMaxSize + 1];
	
	int err = ReadFull(fd, rbuff, 4);

	if (err)
	{
		int lastError = WSAGetLastError();
		if (lastError == 0)
		{
			std::cout << "EOF" << std::endl;
		}

		else
		{
			std::cerr << "ReadFull() error " << lastError << std::endl;
		}

		return err;

	}

	uint32_t len = 0;

	memcpy(&len, rbuff, 4);

	if (len > kMaxSize)
	{
		Msg("Message too long");
		return -1;
	}

	err = ReadFull(fd, &rbuff[4], len);

	if (err)
	{
		int lastError = WSAGetLastError();

		std::cerr << "ReadFull() error " << lastError << std::endl;
		
		return err;
	}

	// Do something
	rbuff[4 + len] = '\0';
	std::cout << "Server says-> " << &rbuff[4] << std::endl;

	return 0;
}