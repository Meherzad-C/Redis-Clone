#pragma once

#include <iostream>     // std::cerr and std::endl
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <cassert>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

class TCP_Client {
private:
    SOCKET fd;
    static const size_t kMaxMsg{ 4096 };

private:
    static void Die(const char* msg);
    static void Msg(const char* msg);

    static int32_t ReadFull(SOCKET fd, char* buff, size_t n);
    static int32_t WriteAll(SOCKET fd, const char* buff, size_t n);

public:
    TCP_Client();
    ~TCP_Client();

    SOCKET GetSocket() const;
    bool ConnectToServer(const char* serverAddress, int port);
    void Send_Message(const char* message);
    void Receive_Message(char* buffer, size_t bufferSize);
    static int32_t QueryServer(SOCKET fd, const char* text);
    // The QueryServer() is simply broken into SendRequest() and ReadRequest()
    static int32_t SendRequest(SOCKET fd, std::vector<std::string>& cmd);
    static int32_t ReadRequest(SOCKET fd);
};
