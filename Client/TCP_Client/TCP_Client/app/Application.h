#pragma once

#include "..\src\TCP_Client.h"

#define PORT 1234

// ==============================
//	Class Declaration
// ==============================

class Application
{
private:
	void InitilizeClient();

public:
	Application();
	~Application();

	void Run();
};