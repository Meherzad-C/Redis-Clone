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

void Application::Run(int argc, char** argv)
{
	InitilizeClient(argc, argv);
}

// ==============================
//	Private Member Functions
// ==============================

void Application::InitilizeClient(int argc, char** argv)
{
	TCP_Client client;
	
	if (!client.ConnectToServer(IP_ADDRESS, PORT))
	{
		std::cerr << "Unable to connect to server, quitting..." << std::endl;
		return;
	}

	SOCKET fd = client.GetSocket();
	
	std::vector<std::string> cmd;
	for (int i = 1; i < argc; ++i) 
	{
		std::cout << argv[i] << std::endl;
		cmd.push_back(argv[i]);
	}

	int32_t err = client.SendRequest(fd, cmd);
	if (err != 0)
	{
		std::cout << "SendRequest() error" << std::endl;
	}

	err = client.ReadRequest(fd);
	if (err != 0)
	{
		std::cout << "ReadRequest() error" << std::endl;
	}
}