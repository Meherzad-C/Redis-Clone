#include "Application.h"

#define IP_ADDRESS "127.0.0.1"
#define PORT 5000

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
	
	if (!client.ConnectToServer(IP_ADDRESS, PORT))
	{
		std::cerr << "Unable to connect to server, quitting..." << std::endl;
		return;
	}

	SOCKET fd = client.GetSocket();
	const char* queryList[3] = { "hello1", "hello2", "hello3" };

	for (int i = 0; i < 3; ++i)
	{
		int err = client.SendRequest(fd, queryList[i]);
		if (err != 0)
		{
			std::cerr << "Falied to SendRequest()" << std::endl;
			return;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		int err = client.ReadRequest(fd);
		if (err != 0)
		{
			std::cerr << "Failed to ReadRequest()" << std::endl;
			return;
		}
	}
}