#include "Window.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <stdexcept>
#include <string>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

Window::Window() : window(nullptr)
{
	assert(glfwInit() == GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	const std::string window_name("Vulkan Playground");
	uint32_t		  window_size_x = 800;
	uint32_t		  window_size_y = 600;
	this->window = glfwCreateWindow(window_size_x, window_size_y, window_name.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	/* Registered before ImGui's GLFW backend is initialized (in VKManager::CreateGui), so that it
	 * detects and chains to this callback instead of being silently overwritten by it. */
	glfwSetScrollCallback(window, scrollCallback);
	if (!this->window)
	{
		throw std::runtime_error("Failed to create GLFW window");
	}
}

Window::~Window() { glfwDestroyWindow(window); }

Window::Window(Window&& other) : window(other.window)
{
	other.window = nullptr;
	/* The GLFW callbacks look up their target via glfwGetWindowUserPointer, so it must be repointed
	 * at this object's address, not left pointing at `other`. */
	if (window)
	{
		glfwSetWindowUserPointer(window, this);
	}
}

Window& Window::operator=(Window&& other)
{
	this->window = other.window;
	other.window = nullptr;
	if (window)
	{
		glfwSetWindowUserPointer(window, this);
	}
	return *this;
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window); }

void Window::pollEvents() const { glfwPollEvents(); }

void Window::waitEvents() const { glfwWaitEvents(); }

GLFWwindow* Window::GetGLFWHandle() const { return window; }

void Window::GetFramebufferSize(int32_t& width, int32_t& height) const
{
	glfwGetFramebufferSize(window, &width, &height);
}

bool Window::GetMouseButtonDown(int button) const { return glfwGetMouseButton(window, button) == GLFW_PRESS; }

void Window::GetCursorPos(double& x, double& y) const { glfwGetCursorPos(window, &x, &y); }

double Window::ConsumeScrollDeltaY() const
{
	double delta = scrollDeltaY;
	scrollDeltaY = 0.0;
	return delta;
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	(void)width;
	(void)height;
	auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	w->windowResized = true;
}

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	(void)xoffset;
	auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	w->scrollDeltaY += yoffset;
}