#pragma once

#include "..\src\TCP_Client.h"

#define PORT 1234

// ==============================
//	Class Declaration
// ==============================

class Application
{
private:
	void InitilizeClient(int argc, char** argv); // TODO: find a better way for passing arguements from main()

public:
	Application();
	~Application();

	void Run(int argc, char** argv);
};