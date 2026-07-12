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

static const std::vector<Mupfel::Transform> vertices = {
	{{0.0f, -0.5f}, {1.0f, 0.0f}},
	{{0.4330f, 0.25f}, {0.0f, 1.0f}},
	{{-0.4330f, 0.25f}, {0.0f, 0.0f}}};

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

	/* Fill the World */

	for (uint32_t i = 0; i < vertices.size(); i++)
	{
		Entity e = world.registry.CreateEntity();
		world.entities.push_back(e);

		world.registry.AddComponent<Transform>(e, {vertices[i]});
		world.registry.AddComponent<Texture>(e, {});
	}
}

void Mupfel::Application::MainLoop()
{
	while (!window->shouldClose())
	{
		window->pollEvents();
		frame_counter++;

		constexpr float countsPerRotation = 30000.0f;
		constexpr float twoPi = 6.28318530718f;

		// counter % 150 sorgt dafür, dass sich der Winkel nach einer vollen
		// Umdrehung wieder bei 0 fortsetzt, statt unbegrenzt zu wachsen.
		float frame = static_cast<float>(frame_counter % static_cast<uint32_t>(countsPerRotation)) / countsPerRotation;
		float angle = frame * twoPi;

		float cosA = std::cos(angle);
		float sinA = std::sin(angle);

		/* World Updates */
		auto rect_view = world.registry.view<Mupfel::Transform>();

		uint32_t index = 0;
		for (auto [entity, t] : rect_view)
		{
			t.pos.x = vertices[index].pos.x * cosA - vertices[index].pos.y * sinA;
			t.pos.y = vertices[index].pos.x * sinA + vertices[index].pos.y * cosA;
			world.registry.SetComponent<Transform>(entity, t);
			index++;
		}

		/* Sync from World to GPU buffers will be done in the Renderer */

		renderer.RenderNextFrame(world, device.value(), window.value());
	}
}
