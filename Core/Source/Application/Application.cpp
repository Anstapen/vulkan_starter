#include "Application.h"
#include <assert.h>

Mupfel::Application::Application(
	const std::string& in_name)
	: name(in_name), renderer(), logger(), device()
{
}

void Mupfel::Application::Run()
{
	Init();
	MainLoop();
	CleanUp();
}

void Mupfel::Application::Init()
{
	/* Create a Logger for the Application */
	logger = Logger::Create("App");
	logger->info("Initializing");
	window = Window();
	device = Ping::Device(Ping::DeviceSpecification());
}

void Mupfel::Application::MainLoop()
{
	while (!window->shouldClose())
	{
		window->pollEvents();
	}
}

void Mupfel::Application::CleanUp()
{
}
