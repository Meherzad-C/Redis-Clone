#include "Application.h"

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
	InitilizeServer();
}

// ==============================
//	Private Member Functions
// ==============================

void Application::InitilizeServer()
{
	TCP_Server server(PORT);
	server.Start();
}