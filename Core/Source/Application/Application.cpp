#include "Application.h"
#include <assert.h>
#include "Ping/Ping.h"

#include "Ping/SwapChain.h"
#include "Ping/Pipeline.h"

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

	/* Initialize the Graphics library */
	if (!Ping::Init())
	{
		logger->error("Failed to initialize Ping");
		assert(false);
	}
	window = Window();
	device = Ping::Device(Ping::DeviceSpecification(), window.value());
	Ping::SwapChain swapChain = device->CreateSwapChain(window.value());
	Ping::Pipeline pipeline = device->CreatePipeline(Ping::PipelineSpecification{ "Shaders/slang.spv" }, swapChain);
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
	Ping::Shutdown();
}
