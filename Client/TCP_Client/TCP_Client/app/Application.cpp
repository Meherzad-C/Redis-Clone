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
	TCP_Client client;
	client.ConnectToServer(IP_ADDRESS, PORT);

	/*const char* message = "hello";
	client.Send_Message(message);

	char rbuf[64] = {};
	client.Receive_Message(rbuf, sizeof(rbuf));
	std::cout << "server says: " << rbuf << std::endl;*/

	int err = client.QueryServer(client.GetSocket(), "hello1");
	if (err)
	{
		std::cout << "Query1 failed " << std::endl;
	}

	err = client.QueryServer(client.GetSocket(), "hello2");
	if (err)
	{
		std::cout << "Query2 failed " << std::endl;
	}

	err = client.QueryServer(client.GetSocket(), "hello3");
	if (err)
	{
		std::cout << "Query3 failed " << std::endl;
	}
}