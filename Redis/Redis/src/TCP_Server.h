#pragma once

#include <WinSock2.h>
#include <iostream>
#include <cassert>

#pragma comment(lib, "Ws2_32.lib")

class TCP_Server
{
private:
	int port;
	SOCKET fd;
    static const size_t kMaxSize{ 4096 };

private:
    void SetupSocket();
    void BindSocket();
    void ListenForConnections();
    void HandleConnections();
    void DoSomething(SOCKET connfd);
    static int32_t OneRequest(SOCKET connfd);
    static int32_t ReadFull(SOCKET fd, char* buff, size_t n);
    static int32_t WriteFull(SOCKET fd, const char* buff, size_t n);

    static void Msg(const char* msg);
    static void Die(const char* msg);

public:
    TCP_Server(int port);
    ~TCP_Server();
    void Start();
};