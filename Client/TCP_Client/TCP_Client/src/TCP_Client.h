#pragma once

#include <iostream>     // std::cerr and std::endl
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class TCPClient {
private:
    SOCKET fd;

private:
    void Die(const char* msg);

public:
    TCPClient();
    ~TCPClient();

    void ConnectToServer(const char* serverAddress, int port);
    void Send_Message(const char* message);
    void Receive_Message(char* buffer, size_t bufferSize);
};
