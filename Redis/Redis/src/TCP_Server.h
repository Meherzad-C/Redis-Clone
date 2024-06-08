#pragma once

#include <WinSock2.h>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <map>

#pragma comment(lib, "Ws2_32.lib")

class TCP_Server
{
private:
	int port;
	SOCKET fd;
    static const size_t kMaxMsg{ 4096 };
    static const size_t kMaxArgs{ 1024 };

    enum
    {
        STATE_REQUEST = 0,
        STATE_RESPONSE = 1,
        STATE_END = 2
    };

    enum
    {
        RESPONSE_OK = 0,
        RESPONSE_ERROR = 1,
        RESPONSE_NOT_FOUND = 2
    };

    typedef struct Conn
    {
        SOCKET fd = INVALID_SOCKET;
        uint32_t state = 0; // either STATE_REQUEST or STATE_RESPONSE
        // buffer for reading
        size_t rbuff_size = 0;
        uint8_t rbuff[4 + kMaxMsg];
        // buffer for writing
        size_t wbuff_size = 0;
        size_t wbuff_sent = 0;
        uint8_t wbuff[4 + kMaxMsg];
    }Conn;

    std::vector<Conn*> fd2conn;

    // The data structure for the key space. 
    // This is just a placeholder until we implement a hashtable.
    static std::map<std::string, std::string> g_map;

private:
    void SetupSocket();
    void BindSocket();
    void ListenForConnections();
    static void Msg(const char* msg);
    static void Die(const char* msg);
    void HandleConnections();
    void DoSomething(SOCKET connfd);
    bool TryOneRequest(Conn* conn);
    void StateRequest(Conn* conn);
    void StateResponse(Conn* conn);
    bool TryFillBuffer(Conn* conn);
    bool TryFlushBuffer(Conn* conn);
    void ConnectionIO(Conn* conn);
    static void SetNonBlockingFD(SOCKET fd);
    static void ConnPut(std::vector<Conn*>& fd2conn, Conn* conn);
    static int32_t AcceptNewConnection(std::vector<Conn*>& fd2conn, int fd);
    static int32_t OneRequest(SOCKET connfd);
    static int32_t ReadFull(SOCKET fd, char* buff, size_t n);
    static int32_t WriteAll(SOCKET fd, const char* buff, size_t n);
    static int32_t ParseRequest(const uint8_t* data, size_t len, std::vector<std::string>& out);
    static uint32_t DoGet(const std::vector<std::string>& cmd, uint8_t* res, uint32_t* reslen);
    static uint32_t DoSet(const std::vector<std::string>& cmd, uint8_t* res, uint32_t* reslen);
    static uint32_t DoDel(const std::vector<std::string>& cmd, uint8_t* res, uint32_t* reslen);
    static int32_t DoRequest(const uint8_t* req, uint32_t reqlen, uint32_t* rescode, uint8_t* res, uint32_t* reslen);

public:
    TCP_Server(int port);
    ~TCP_Server();
    void Start();
};