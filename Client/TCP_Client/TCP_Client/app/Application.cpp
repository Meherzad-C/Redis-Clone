#include "Application.h"

#define IP_ADDRESS "127.0.0.1"
#define PORT 1234

// ==============================
//	Constructors and Destructors
// ==============================

Application::Application()
{

}

Application::~Application()
{

}

// ==============================
//	Run Function
// ==============================

void Application::Run()
{
	InitilizeClient();
}

// ==============================
//	Private Member Functions
// ==============================

void Application::InitilizeClient()
{
	TCPClient client;
	client.ConnectToServer(IP_ADDRESS, PORT);

	const char* message = "hello";
	client.Send_Message(message);

	char rbuf[64] = {};
	client.Receive_Message(rbuf, sizeof(rbuf));
	std::cout << "server says: " << rbuf << std::endl;
}