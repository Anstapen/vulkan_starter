#pragma once
#include "Logger/Logger.h"
#include "Renderer/Renderer.h"
#include "Window/Window.h"
#include <cstdint>
#include <optional>
#include <string>

#include "World.h"

/* Graphics API */
#include "Ping/Device.h"

namespace Mupfel
{
/**
 * Top-level entry point for a Mupfel-based app: owns the `Window`, `Ping::Device`, `Renderer`, and
 * `World`, and drives the `Init()` / `MainLoop()` lifecycle.
 */
class Application
{
public:
	/** Constructs an application named `in_name`. Call `Run()` to actually initialize and start it. */
	Application(const std::string& in_name);

	/** Waits for the device to go idle and shuts down the Ping backend. */
	~Application();

public:
	/** Initializes the window/device/renderer/world, then runs the main loop until the window closes. */
	void Run();

private:
	/** Creates the Ping backend, window, and device; initializes the renderer; populates the initial `World`. */
	void Init();

	/** Polls window events and renders frames until the window is closed. */
	void MainLoop();

private:
	/** Application name passed to the constructor. */
	const std::string name;
	/** Logger created in `Init()`. */
	Logger::SafeLoggerPtr logger;
	/** The RHI device; empty until `Init()` runs. */
	std::optional<Ping::Device> device;
	/** The application window; empty until `Init()` runs. */
	std::optional<Window> window;
	/** Owns the swapchain/pipeline/command buffers for rendering. */
	Renderer renderer;
	/** ECS registry and event system for this application. */
	World world;
	/** Incremented once per `MainLoop()` iteration. */
	uint64_t frame_counter = 0;
};

} // namespace Mupfel