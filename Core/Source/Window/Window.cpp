#include "Window.h"
#include <GLFW/glfw3.h>
#include <string>
#include <cassert>
#include <stdexcept>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

Window::Window() : window(nullptr)
{
	assert(glfwInit() == GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	const std::string window_name("Vulkan Playground");
	uint32_t window_size_x = 800;
	uint32_t window_size_y = 600;
	this->window = glfwCreateWindow(window_size_x, window_size_y, window_name.c_str(),
		nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	if (!this->window)
	{
		throw std::runtime_error("Failed to create GLFW window");
	}
}

Window::~Window()
{
	glfwDestroyWindow(window);
}

Window::Window(Window&& other) : window(other.window)
{
	other.window = nullptr;
}

Window& Window::operator=(Window&& other)
{
	this->window = other.window;
	other.window = nullptr;
	return *this;
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(window);
}

void Window::pollEvents() const
{
	glfwPollEvents();
}

void Window::waitEvents() const
{
	glfwWaitEvents();
}

GLFWwindow* Window::GetGLFWHandle() const
{
	return window;
}

void Window::GetFramebufferSize(int32_t& width, int32_t& height) const
{
	glfwGetFramebufferSize(window, &width, &height);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	w->windowResized = true;
}