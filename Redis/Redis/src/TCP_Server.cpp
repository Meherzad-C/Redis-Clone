#include "TCP_Server.h"

// ==============================
//	Marcros, 
//	static member variables
//	& other junk
// ==============================

#define CMD_IS(word, cmd) (_stricmp((word).c_str(), (cmd)) == 0)

TCP_Server::gData TCP_Server::gdata_;
std::vector<TCP_Server::Conn*> TCP_Server::fd2conn;

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

	SetNonBlockingFD(this->fd);
}

void TCP_Server::HandleConnections()
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
		uint32_t timeout_ms = NextTimerMs();
		timeout.tv_sec = timeout_ms / 1000; // Whole seconds
		timeout.tv_usec = (timeout_ms % 1000) * 1000; // Remaining microseconds

		int rv = select(static_cast<int>(maxfd + 1), &readfds, &writefds, &exceptfds, &timeout);
		if (rv == SOCKET_ERROR)
		{
			Die("select()");
		}

		/*if (rv == 0)
		{
			continue;
		}*/

		if (FD_ISSET(this->fd, &readfds))
		{
			AcceptNewConnection(this->fd);
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
					ConnDone(pConn);
					it = this->fd2conn.erase(it);
					continue;
				}
			}
			++it;
		}
		// handle timers
		ProcessTimers();
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

	// parse the request
	std::vector<std::string> cmd;
	if (0 != ParseRequest(&conn->rbuff[4], len, cmd)) 
	{
		Msg("bad req");
		conn->state = STATE_END;
		return false;
	}

	// got one request, generate the response.
	std::string out;
	DoRequest(cmd, out);

	// pack the response into the buffer
	if (4 + out.size() > kMaxMsg) {
		out.clear();
		OutError(out, ERROR_TOO_BIG, "response is too big");
	}
	uint32_t wlen = (uint32_t)out.size();

	memcpy(&conn->wbuff[0], &wlen, 4);
	memcpy(&conn->wbuff[4], out.data(), out.size());
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
	// woke up by poll, update the idle timer
	// by moving conn to the end of the list.
	conn->idle_start = GetMonotonicUsec();
	conn->idleList.Detach();
	gdata_.idleList.Insert_Before(&conn->idleList);

	// do the work
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

int32_t TCP_Server::AcceptNewConnection(int fd)
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

	conn->idle_start = GetMonotonicUsec();
	gdata_.idleList.Insert_Before(&conn->idleList);
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

uint64_t TCP_Server::GetMonotonicUsec()
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);

	// Convert to microseconds
	return (counter.QuadPart * 1000000) / frequency.QuadPart;
}

uint32_t TCP_Server::NextTimerMs()
{
	uint64_t now_us = GetMonotonicUsec();
	uint64_t next_us = (uint64_t)-1;

	// idle timers
	if (!gdata_.idleList.Empty()) 
	{
		Conn* next = CONTAINER_OF(gdata_.idleList.next, Conn, idleList);
		next_us = next->idle_start + kIdleTimeout_ms * 1000;
	}

	// ttl timers
	if (!gdata_.heap.Empty() && gdata_.heap.AccessElement(0).val < next_us) 
	{
		next_us = gdata_.heap.AccessElement(0).val;
	}

	if (next_us == (uint64_t)-1) 
	{
		// no timer, the value doesn't matter
		return 10000;   
	}

	if (next_us <= now_us) 
	{
		// missed?
		return 0;
	}
	return (uint32_t)((next_us - now_us) / 1000);
}

void TCP_Server::ConnDone(Conn* conn)
{
	fd2conn[conn->fd] = nullptr;
	closesocket(conn->fd);
	conn->idleList.Detach();
	delete conn;
}

void TCP_Server::ProcessTimers()
{
	// the extra 1000us is for the ms resolution of poll()
	uint64_t now_us = GetMonotonicUsec() + 1000;

	// idle timers
	while (!gdata_.idleList.Empty())
	{
		Conn* next = CONTAINER_OF(gdata_.idleList.next, Conn, idleList);
		uint64_t next_us = next->idle_start + kIdleTimeout_ms * 1000;
		if (next_us >= now_us) 
		{
			// not ready
			break;
		}

		printf("removing idle connection: %d\n", next->fd);
		ConnDone(next);
	}

	// TTL timers
	const size_t k_max_works = 2000;
	size_t nworks = 0;
	while (!gdata_.idleList.Empty() && gdata_.heap.AccessElement(0).val < now_us) 
	{
		Entry* ent = CONTAINER_OF(gdata_.heap.AccessElement(0).ref, Entry, heapIdx);
		HNode* node = gdata_.db.HM_Pop(&ent->node, &PointerEqual);
		assert(node == &ent->node);
		EntryDelete(ent);
		if (nworks++ >= k_max_works) 
		{
			// don't stall the server if too many keys are expiring at once
			break;
		}
	}
}

// set or remove the TTL
void TCP_Server::EntrySetTTL(Entry* ent, int64_t ttl_ms)
{
	if (ttl_ms < 0 && ent->heapIdx != (size_t)-1) 
	{
		// erase an item from the heap
		// by replacing it with the last item in the array.
		size_t pos = ent->heapIdx;
		gdata_.heap.AccessElement(pos) = gdata_.heap.Back();
		gdata_.heap.Pop();
		if (pos < gdata_.heap.Size()) 
		{
			gdata_.heap.Update(pos);
		}
		ent->heapIdx = -1;
	}
	else if (ttl_ms >= 0) 
	{
		size_t pos = ent->heapIdx;
		if (pos == (size_t)-1) 
		{
			// add an new item to the heap
			HeapItem item;
			item.ref = &ent->heapIdx;
			gdata_.heap.Push(item);
			pos = gdata_.heap.Size() - 1;
		}
		gdata_.heap.AccessElement(pos).val = GetMonotonicUsec() + (uint64_t)ttl_ms * 1000;
		gdata_.heap.Update(pos);
	}
}

void TCP_Server::DoExpire(std::vector<std::string>& cmd, std::string& out)
{
	int64_t ttl_ms = 0;
	if (!StringToInt(cmd[2], ttl_ms))
	{
		return OutError(out, ERROR_ARG, "expect int64");
	}

	Entry key;
	key.key.swap(cmd[1]);
	key.node.hcode = StringHash((uint8_t*)key.key.data(), key.key.size());

	HNode* node = gdata_.db.HM_Lookup(&key.node, &EntryEq);
	if (node) 
	{
		Entry* ent = CONTAINER_OF(node, Entry, node);
		EntrySetTTL(ent, ttl_ms);
	}
	return OutInt(out, node ? 1 : 0);
}

void TCP_Server::DoTTL(std::vector<std::string>& cmd, std::string& out)
{
	Entry key;
	key.key.swap(cmd[1]);
	key.node.hcode = StringHash((uint8_t*)key.key.data(), key.key.size());

	HNode* node = gdata_.db.HM_Lookup(&key.node, &EntryEq);
	if (!node) 
	{
		return OutInt(out, -2);
	}

	Entry* ent = CONTAINER_OF(node, Entry, node);
	if (ent->heapIdx == (size_t)-1) 
	{
		return OutInt(out, -1);
	}

	uint64_t expire_at = gdata_.heap.AccessElement(ent->heapIdx).val;
	uint64_t now_us = GetMonotonicUsec();
	return OutInt(out, expire_at > now_us ? (expire_at - now_us) / 1000 : 0);
}

bool TCP_Server::EntryEq(HNode* lhs, HNode* rhs)
{
	Entry* le = CONTAINER_OF(lhs, Entry, node);
	Entry* re = CONTAINER_OF(rhs, Entry, node);
	return le->key == re->key;
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

void TCP_Server::CbScan(HNode* node, void* arg)
{
	std::string& out = *(std::string*)arg;
	OutString(out, CONTAINER_OF(node, Entry, node)->key);
}

void TCP_Server::OutNil(std::string& out) 
{
	out.push_back(SERIALIZATION_NIL);
}

void TCP_Server::OutString(std::string& out, const char* s, size_t size)
{
	out.push_back(SERIALIZATION_STRING);
	uint32_t len = (uint32_t)size;
	out.append((char*)&len, 4);
	out.append(s, len);
}

void TCP_Server::OutString(std::string& out, const std::string& val)
{
	return OutString(out, val.data(), val.size());
}

void TCP_Server::DoKeys(std::vector<std::string>& cmd, std::string& out)
{
	(void)cmd;
	OutArray(out, (uint32_t)gdata_.db.HM_Size());
	gdata_.db.HM_Scan(HMap::HTableType::PRIMARY_HT1, &CbScan, &out);
	gdata_.db.HM_Scan(HMap::HTableType::SECONDARY_HT2, &CbScan, &out);
}

void TCP_Server::OutInt(std::string& out, int64_t val) 
{
	out.push_back(SERIALIZATION_INT);
	out.append((char*)&val, 8);
}

void TCP_Server::OutDouble(std::string& out, double val)
{
	out.push_back(SERIALIZATION_DOUBLE);
	out.append((char*)&val, 8);
}

void TCP_Server::OutError(std::string& out, int32_t code, const std::string& msg) 
{
	out.push_back(SERIALIZATION_ERROR);
	out.append((char*)&code, 4);
	uint32_t len = (uint32_t)msg.size();
	out.append((char*)&len, 4);
	out.append(msg);
}

void TCP_Server::OutArray(std::string& out, uint32_t n) 
{
	out.push_back(SERIALIZATION_ARRAY);
	out.append((char*)&n, 4);
}

void* TCP_Server::BeginArray(std::string& out) 
{
	out.push_back(SERIALIZATION_ARRAY);
	out.append("\0\0\0\0", 4);          // filled in end_arr()
	return (void*)(out.size() - 4);     // the `ctx` arg
}

void TCP_Server::EndArray(std::string& out, void* ctx, uint32_t n) 
{
	size_t pos = (size_t)ctx;
	assert(out[pos - 1] == SERIALIZATION_ARRAY);
	memcpy(&out[pos], &n, 4);
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

void TCP_Server::DoGet(std::vector<std::string>& cmd, std::string& out)
{
	Entry key;
	// gData gdata_;
	key.key.swap(cmd[1]);
	key.node.hcode = StringHash((uint8_t*)key.key.data(), key.key.size());

	HNode* node = gdata_.db.HM_Lookup(&key.node, &EntryEq);
	if (!node) 
	{
		return OutNil(out);
	}

	Entry* ent = CONTAINER_OF(node, Entry, node);
	if (ent->type != T_STRING) 
	{
		return OutError(out, ERROR_TYPE, "expect string type");
	}
	return OutString(out, ent->val);
}

void TCP_Server::DoSet(std::vector<std::string>& cmd, std::string& out)
{
	Entry key;
	// gData gdata_;
	key.key.swap(cmd[1]);
	key.node.hcode = StringHash((uint8_t*)key.key.data(), key.key.size());

	HNode* node = gdata_.db.HM_Lookup(&key.node, &EntryEq);
	if (node) 
	{
		Entry* ent = CONTAINER_OF(node, Entry, node);
		if (ent->type != T_STRING) 
		{
			return OutError(out, ERROR_TYPE, "expect string type");
		}
		ent->val.swap(cmd[2]);
	}
	else {
		Entry* ent = new Entry();
		ent->key.swap(key.key);
		ent->node.hcode = key.node.hcode;
		ent->val.swap(cmd[2]);
		gdata_.db.HM_Insert(& ent->node);
	}
	return OutNil(out);
}

void TCP_Server::DoDel(std::vector<std::string>& cmd, std::string& out)
{
	Entry key;
	// gData gdata_;
	key.key.swap(cmd[1]);
	key.node.hcode = StringHash((uint8_t*)key.key.data(), key.key.size());

	HNode* node = gdata_.db.HM_Pop(&key.node, &EntryEq);
	if (node) 
	{
		EntryDelete(CONTAINER_OF(node, Entry, node));
	}

	return OutInt(out, node ? 1 : 0);
}

void TCP_Server::DoRequest(std::vector<std::string>& cmd, std::string& out)
{
	if (cmd.size() == 1 && CMD_IS(cmd[0], "keys")) 
	{
		DoKeys(cmd, out);
	}
	else if (cmd.size() == 2 && CMD_IS(cmd[0], "get")) 
	{
		DoGet(cmd, out);
	}
	else if (cmd.size() == 3 && CMD_IS(cmd[0], "set")) 
	{
		DoSet(cmd, out);
	}
	else if (cmd.size() == 2 && CMD_IS(cmd[0], "del")) 
	{
		DoDel(cmd, out);
	}
	else if (cmd.size() == 3 && CMD_IS(cmd[0], "pexpire")) 
	{
		DoExpire(cmd, out);
	}
	else if (cmd.size() == 2 && CMD_IS(cmd[0], "pttl"))
	{
		DoTTL(cmd, out);
	}
	else if (cmd.size() == 4 && CMD_IS(cmd[0], "zadd"))
	{
		DoZadd(cmd, out);
	}
	else if (cmd.size() == 3 && CMD_IS(cmd[0], "zrem"))
	{
		DoZrem(cmd, out);
	}
	else if (cmd.size() == 3 && CMD_IS(cmd[0], "zscore"))
	{
		DoZscore(cmd, out);
	}
	else if (cmd.size() == 6 && CMD_IS(cmd[0], "zquery"))
	{
		DoZquery(cmd, out);
	}
	else 
	{
		// cmd is not recognized
		OutError(out, ERROR_UNKNOWN, "Unknown cmd");
	}
}

void TCP_Server::EntryDelete(Entry* ent) 
{
	switch (ent->type) 
	{
		case T_ZSET:
			ent->zset->Dispose();
			delete ent->zset;
			break;
	}
	EntrySetTTL(ent, -1);
	delete ent;
}

bool TCP_Server::ExpectZset(std::string& out, std::string& s, Entry** ent)
{
	Entry key;
	key.key.swap(s);
	key.node.hcode = StringHash((uint8_t*)key.key.data(), key.key.size());
	HNode* hnode = gdata_.db.HM_Lookup(&key.node, &EntryEq);
	if (!hnode) 
	{
		OutNil(out);
		return false;
	}

	*ent = CONTAINER_OF(hnode, Entry, node);
	if ((*ent)->type != T_ZSET) 
	{
		OutError(out, ERROR_TYPE, "expect zset");
		return false;
	}
	return true;
}

// zadd set score name
void TCP_Server::DoZadd(std::vector<std::string>& cmd, std::string& out)
{
	double score = 0;
	if (!StringToDouble(cmd[2], score)) 
	{
		return OutError(out, ERROR_ARG, "expect fp number");
	}

	// look up or create the zset
	Entry key;
	key.key.swap(cmd[1]);
	key.node.hcode = StringHash((uint8_t*)key.key.data(), key.key.size());
	HNode* hnode = gdata_.db.HM_Lookup(&key.node, &EntryEq);

	Entry* ent = NULL;
	if (!hnode) 
	{
		ent = new Entry();
		ent->key.swap(key.key);
		ent->node.hcode = key.node.hcode;
		ent->type = T_ZSET;
		ent->zset = new ZSet();
		gdata_.db.HM_Insert(&ent->node);
	}
	else 
	{
		ent = CONTAINER_OF(hnode, Entry, node);
		if (ent->type != T_ZSET) 
		{
			return OutError(out, ERROR_TYPE, "expect zset");
		}
	}

	// add or update the tuple
	const std::string& name = cmd[3];
	bool added = ent->zset->Add(name.data(), name.size(), score);
	return OutInt(out, (int64_t)added);
}

// zrem zset name
void TCP_Server::DoZrem(std::vector<std::string>& cmd, std::string& out)
{
	Entry* ent = NULL;
	if (!ExpectZset(out, cmd[1], &ent)) 
	{
		return;
	}

	const std::string& name = cmd[2];
	ZNode* znode = ent->zset->Pop(name.data(), name.size());
	if (znode) 
	{
		znode->Destroy(znode);
	}
	return OutInt(out, znode ? 1 : 0);
}

// zscore zset name
void TCP_Server::DoZscore(std::vector<std::string>& cmd, std::string& out)
{
	Entry* ent = NULL;
	if (!ExpectZset(out, cmd[1], &ent)) 
	{
		return;
	}

	const std::string& name = cmd[2];
	ZNode* znode = ent->zset->Lookup(name.data(), name.size());
	return znode ? OutDouble(out, znode->score) : OutNil(out);
}

// zquery zset score name offset limit
void TCP_Server::DoZquery(std::vector<std::string>& cmd, std::string& out)
{
	// parse args
	double score = 0;
	if (!StringToDouble(cmd[2], score)) 
	{
		return OutError(out, ERROR_ARG, "expect fp number");
	}
	const std::string& name = cmd[3];
	int64_t offset = 0;
	int64_t limit = 0;
	if (!StringToInt(cmd[4], offset)) 
	{
		return OutError(out, ERROR_ARG, "expect int");
	}
	if (!StringToInt(cmd[5], limit)) 
	{
		return OutError(out, ERROR_ARG, "expect int");
	}

	// get the zset
	Entry* ent = NULL;
	if (!ExpectZset(out, cmd[1], &ent)) 
	{
		if (out[0] == SERIALIZATION_NIL) 
		{
			out.clear();
			OutArray(out, 0);
		}
		return;
	}

	// look up the tuple
	if (limit <= 0) 
	{
		return OutArray(out, 0);
	}
	ZNode* znode = ent->zset->Query(score, name.data(), name.size());
	znode = ent->zset->Offset(znode, offset);

	// output
	void* arr = BeginArray(out);
	uint32_t n = 0;
	while (znode && (int64_t)n < limit) 
	{
		OutString(out, znode->name, znode->length);
		OutDouble(out, znode->score);
		znode = ent->zset->Offset(znode, +1);
		n += 2;
	}
	EndArray(out, arr, n);
}