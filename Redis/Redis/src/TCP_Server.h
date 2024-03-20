#pragma once

#include <WinSock2.h>
#include <iostream>
#include <cassert>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

class TCP_Server
{
private:
	int port;
	SOCKET fd;
    static const size_t kMaxSize{ 4096 };
    std::vector<struct Conn*> fd2conn;

    enum
    {
        STATE_REQUEST = 0,
        STATE_RESPONSE = 1,
        STATE_END = 2
    };

    typedef struct
    {
        SOCKET fd = INVALID_SOCKET;
        uint32_t state = 0;
        size_t rbuff_size = 0;
        uint8_t rbuff[4 + kMaxSize];
        size_t wbuff_size = 0;
        size_t wbuff_sent = 0;
        uint8_t wbuff[4 + kMaxSize];
    }Conn;


private:
    void SetupSocket();
    void BindSocket();
    void ListenForConnections();
    void HandleConnections();
    void SetNonBlockingFD(SOCKET fd);
    void DoSomething(SOCKET connfd);
    void ConnPut(std::vector<Conn*>& fd2conn, Conn* conn);
    bool TryOneRequest(Conn* conn);
    void StateRequest(Conn* conn);
    void StateResponse(Conn* conn);
    bool TryFillBuffer(Conn* conn);
    bool TryFlushBuffer(Conn* conn);
    void Connection_IO(Conn* conn);
    static int32_t AcceptNewConnection(std::vector<Conn*>& fd2conn, int fd);
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