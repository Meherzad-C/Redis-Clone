#include "TCP_Client.h"

// ==============================
//	Constructors and Destructors
// ==============================

TCPClient::TCPClient()
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

TCPClient::~TCPClient()
{
	closesocket(this->fd);
	WSACleanup();
}

// ==============================
//	Public Member Functions
// ==============================

void TCPClient::ConnectToServer(const char* serverAddress, int port)
{
	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (inet_pton(AF_INET, serverAddress, &addr.sin_addr) <= 0)
	{
		Die("inet_pton() failed");
	}

	int rv = connect(this->fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (rv < SOCKET_ERROR)
	{
		Die("connect() failed");
	}
}

void TCPClient::Send_Message(const char* message)
{
	size_t msgLen = strlen(message);
	send(this->fd, message, msgLen, 0);
}

void TCPClient::Receive_Message(char* buffer, size_t bufferSize)
{
	// buffer -1 is used directly in the recv() so the received data is null-terminated to ensure proper string handling.
	size_t n = recv(this->fd, buffer, bufferSize - 1, 0);

	if (n == SOCKET_ERROR)
	{
		Die("recv() failed");
	}

	buffer[n] = '\0';
}

void TCPClient::Die(const char* msg)
{
	int errorMsg = WSAGetLastError();
	std::cerr << "[" << errorMsg << "] " << msg << std::endl;

	exit(EXIT_FAILURE);
}
