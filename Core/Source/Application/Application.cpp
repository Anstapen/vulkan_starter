#include "Application.h"
#include <assert.h>
#include "Ping/Ping.h"

#include "Ping/SwapChain.h"
#include "Ping/Pipeline.h"
#include "Ping/CommandBuffer.h"

Mupfel::Application::Application(
	const std::string& in_name)
	: name(in_name), renderer(), logger(), device()
{
}

Mupfel::Application::~Application()
{
	device.value().WaitForCommands();
	Ping::Shutdown();
}

void Mupfel::Application::Run()
{
	Init();
	MainLoop();
}

void Mupfel::Application::Init()
{
	/* Create a Logger for the Application */
	logger = Logger::Create("App");
	logger->info("Initializing");

	/* Initialize the Graphics library */
	if (!Ping::Init())
	{
		logger->error("Failed to initialize Ping");
		assert(false);
	}
	window = Window();
	device = Ping::Device(Ping::DeviceSpecification(), window.value());
	renderer.Init(device.value(), window.value());
}

void Mupfel::Application::MainLoop()
{
	while (!window->shouldClose())
	{
		window->pollEvents();
		renderer.RenderNextFrame(device.value(), window.value());
	}
}
