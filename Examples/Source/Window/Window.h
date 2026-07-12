#pragma once
#include <cstdint>

/* forward declarations */
struct GLFWwindow;
typedef struct GLFWwindow GLFWwindow;

/** RAII-owned GLFW window (800x600, resizable, no client API since Vulkan creates its own surface). */
class Window
{
public:
	/**
	 * Creates the GLFW window.
	 * @throws std::runtime_error if GLFW fails to create the window.
	 */
	Window();
	virtual ~Window();
	Window(const Window& other) = delete;
	/** Move-constructs from `other`, taking over its GLFW handle. */
	Window(Window&& other);
	Window& operator=(const Window& other) = delete;
	/** Move-assigns from `other`, taking over its GLFW handle. */
	Window& operator=(Window&& other);

public:
	/** Whether the user has requested the window be closed. */
	bool shouldClose() const;

	/** Processes pending window/input events. Call once per frame from the main loop. */
	void pollEvents() const;

	/**
	 * Blocks until at least one event is available, then processes pending events (used while
	 * minimized, where the framebuffer size is zero — see `Ping::SwapChain::Recreate`).
	 */
	void waitEvents() const;

	/** The underlying GLFW window handle, for backend code (e.g. `glfwCreateWindowSurface`) that needs it. */
	GLFWwindow* GetGLFWHandle() const;

	/** Writes the window's current framebuffer size (in pixels) to `width`/`height`. */
	void GetFramebufferSize(int32_t& width, int32_t& height) const;

public:
	/**
	 * Set to `true` by the GLFW framebuffer-resize callback. Not currently read anywhere — resize
	 * handling instead happens via the out-of-date/suboptimal results from `SwapChain::AcquireNextImage`/`Present`.
	 */
	volatile bool windowResized = false;

private:
	/** Owning GLFW window handle; destroyed via `glfwDestroyWindow`. */
	GLFWwindow* window;
};