#include "Application.h"
#include "Ping/Ping.h"
#include <assert.h>

#include "Ping/CommandBuffer.h"
#include "Ping/Pipeline.h"
#include "Ping/SwapChain.h"

#include "ECS/Components/Texture.h"
#include "ECS/Components/Transform.h"

Mupfel::Application::Application(const std::string& in_name) : name(in_name), renderer(), logger(), device(), world() {}

Mupfel::Application::~Application()
{
	device.value().WaitForCommands();

	/* TODO: there may be GLFW services called after glfwTerminate has been called... */
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
	device = Ping::Device(Ping::DeviceSpecification(), window.value().GetGLFWHandle());
	renderer.Init(device.value(), window.value());

	for (uint32_t k = 0; k < 100; k++)
	{
		for (uint32_t i = 0; i < 100; i++)
		{
			Entity	  e = world.registry.CreateEntity();
			Transform t;
			t.pos_x = static_cast<float>(k);
			t.pos_y = static_cast<float>(i);
			t.pos_z = 0.0f;
			t.scale_x = 0.5f;
			t.scale_y = 0.5f;
			t.rotation = 0.0f;
			world.registry.AddComponent<Transform>(e, t);
		}
	}
	
	
}

void Mupfel::Application::MainLoop()
{
	while (!window->shouldClose())
	{
		window->pollEvents();

		/* Sync from World to GPU buffers will be done in the Renderer */

		renderer.RenderNextFrame(world, device.value(), window.value());
	}
}
