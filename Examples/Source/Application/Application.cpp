#include "Application.h"
#include "Ping/Ping.h"
#include <algorithm>
#include <assert.h>

#include "Ping/CommandBuffer.h"
#include "Ping/Pipeline.h"
#include "Ping/SwapChain.h"

#include "ECS/Components/Movement.h"
#include "ECS/Components/Texture.h"
#include "ECS/Components/Transform.h"

#include <random>

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

	std::mt19937						   rng(std::random_device{}());
	std::uniform_real_distribution<float> velocity_dist(-kMaxEntityVelocity, kMaxEntityVelocity);

	for (uint32_t k = 0; k < 100; k++)
	{
		for (uint32_t i = 0; i < 100; i++)
		{
			Entity	  e = world.registry.CreateEntity();
			Transform t;
			t.pos_x = static_cast<float>(k/2);
			t.pos_y = static_cast<float>(i/2);
			t.pos_z = 0.0f;
			t.scale_x = 0.5f;
			t.scale_y = 0.5f;
			t.rotation = 0.0f;
			world.registry.AddComponent<Transform>(e, t);

			Movement m;
			m.velocity_x = velocity_dist(rng);
			m.velocity_y = velocity_dist(rng);
			world.registry.AddComponent<Movement>(e, m);
		}
	}
}

void Mupfel::Application::MainLoop()
{
	lastFrameTime = std::chrono::steady_clock::now();

	while (!window->shouldClose())
	{
		window->pollEvents();

		auto  now = std::chrono::steady_clock::now();
		float delta_time = std::chrono::duration<float>(now - lastFrameTime).count();
		lastFrameTime = now;

		/* Update the entity positions */
		UpdateMovement(delta_time);

		/* Sync from World to GPU buffers will be done in the Renderer */

		renderer.RenderNextFrame(world, device.value(), window.value());
	}
}

void Mupfel::Application::UpdateMovement(float delta_time)
{
	constexpr float min_bound = 0.0f;
	constexpr float max_bound = 150.0f;

	for (auto [e, transform, movement] : world.registry.view<Transform, Movement>())
	{
		(void)e;
		transform.pos_x += movement.velocity_x * delta_time;
		transform.pos_y += movement.velocity_y * delta_time;

		/* Bounce off the [min_bound, max_bound] box instead of drifting off, by reflecting the
		 * velocity component perpendicular to whichever edge was crossed. */
		if (transform.pos_x < min_bound || transform.pos_x > max_bound)
		{
			movement.velocity_x *= -1.0f;
			transform.pos_x = std::clamp(transform.pos_x, min_bound, max_bound);
		}
		if (transform.pos_y < min_bound || transform.pos_y > max_bound)
		{
			movement.velocity_y *= -1.0f;
			transform.pos_y = std::clamp(transform.pos_y, min_bound, max_bound);
		}
	}
}
