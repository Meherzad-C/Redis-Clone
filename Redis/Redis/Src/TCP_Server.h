#pragma once

#include <WinSock2.h>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <math.h>
#include <time.h>
#include <windows.h>

// project includes
#include "Common/Utility.h"
#include "Data_Structures/Hashtable/HashMap.h"
#include "Data_Structures/SortedSet/ZSet.h"
#include "Data_Structures/LinkedList/Linkedlist.h"
#include "Data_Structures/Heap/Heap.h"
#include "Modules/Concurrency/ThreadPool.h"

#pragma comment(lib, "Ws2_32.lib")

// TODO: refactor this class. (P1)
class TCP_Server
{
private:
	int port;
	SOCKET fd;
    static const size_t kMaxMsg{ 4096 };
    static const size_t kMaxArgs{ 1024 };
    static const uint64_t kIdleTimeout_ms{ 5 * 1000 };

    enum
    {
        STATE_REQUEST = 0,
        STATE_RESPONSE = 1,
        STATE_END = 2
    };

    enum 
    {
        ERROR_UNKNOWN = 1,
        ERROR_TOO_BIG = 2,
        ERROR_TYPE = 3,
        ERROR_ARG = 4
    };

    enum 
    {
        SERIALIZATION_NIL = 0,
        SERIALIZATION_ERROR = 1,
        SERIALIZATION_STRING = 2,
        SERIALIZATION_INT = 3,
        SERIALIZATION_DOUBLE = 4,
        SERIALIZATION_ARRAY = 5
    };

    enum 
    {
        T_STRING = 0,
        T_ZSET = 1
    };

    typedef struct Conn
    {
        SOCKET fd{ INVALID_SOCKET };
        // either STATE_REQUEST or STATE_RESPONSE
        uint32_t state{ 0 }; 
        // buffer for reading
        size_t rbuff_size{ 0 };
        uint8_t rbuff[4 + kMaxMsg];
        // buffer for writing
        size_t wbuff_size{ 0 };
        size_t wbuff_sent{ 0 };
        uint8_t wbuff[4 + kMaxMsg];
        // timer
        uint64_t idle_start{ 0 };
        // each connection has their own idleList
        DList idleList;
    }Conn;

    static std::vector<Conn*> fd2conn;

    // The data structure for the key space.
    typedef struct gData
    {
        HMap db;
        // timers for idle connections
        DList idleList; 
        // timers for TTL
        Heap heap;
        // the thread pool
        ThreadPool tp{ 4 };
    } gData;

    static gData gdata_;

    // the structure for the key
    typedef struct Entry 
    {
        HNode node;
        std::string key;
        std::string val;
        uint32_t type{ 0 };
        ZSet* zset{ nullptr };
        // for TTLs
        size_t heapIdx{ static_cast<size_t>(-1) };
    }Entry;

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
    static int32_t AcceptNewConnection(int fd);
    static int32_t OneRequest(SOCKET connfd);
    static int32_t ReadFull(SOCKET fd, char* buff, size_t n);
    static int32_t WriteAll(SOCKET fd, const char* buff, size_t n);
    static uint64_t GetMonotonicUsec();
    static uint32_t NextTimerMs();
    static void ConnDone(Conn* conn);
    static void ProcessTimers();
    static void EntrySetTTL(Entry* ent, int64_t ttl_ms);
    static void DoExpire(std::vector<std::string>& cmd, std::string& out);
    static void DoTTL(std::vector<std::string>& cmd, std::string& out);
    static bool EntryEq(HNode* lhs, HNode* rhs);
    static void CbScan(HNode* node, void* arg);
    static void OutNil(std::string& out);
    static void OutString(std::string& out, const char* s, size_t size);
    static void OutString(std::string& out, const std::string& val);
    static void DoKeys(std::vector<std::string>& cmd, std::string& out);
    static void OutInt(std::string& out, int64_t val);
    static void OutDouble(std::string& out, double val);
    static void OutError(std::string& out, int32_t code, const std::string& msg);
    static void OutArray(std::string& out, uint32_t n);
    static void* BeginArray(std::string& out);
    static void EndArray(std::string& out, void* ctx, uint32_t n);
    static int32_t ParseRequest(const uint8_t* data, size_t len, std::vector<std::string>& out);
    static void DoGet(std::vector<std::string>& cmd, std::string& out);
    static void DoSet(std::vector<std::string>& cmd, std::string& out);
    static void DoDel(std::vector<std::string>& cmd, std::string& out);
    static void DoRequest(std::vector<std::string>& cmd, std::string& out);
    static void EntryDeleteAsync(void* arg);
    static void EntryDelete(Entry* ent);
    static void EntryDestroy(Entry* ent);
    static void DoZadd(std::vector<std::string>& cmd, std::string& out);
    static bool ExpectZset(std::string& out, std::string& s, Entry** ent);
    static void DoZrem(std::vector<std::string>& cmd, std::string& out);
    static void DoZscore(std::vector<std::string>& cmd, std::string& out);
    static void DoZquery(std::vector<std::string>& cmd, std::string& out);

public:
    TCP_Server(int port);
    ~TCP_Server();
    void Start();
};