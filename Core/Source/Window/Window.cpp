#include "Window.h"
#include <GLFW/glfw3.h>
#include <string>
#include <cassert>
#include <stdexcept>

Window::Window() : window(nullptr)
{
	assert(glfwInit() == GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	const std::string window_name("Vulkan Playground");
	uint32_t window_size_x = 800;
	uint32_t window_size_y = 600;
	this->window = glfwCreateWindow(window_size_x, window_size_y, window_name.c_str(),
		nullptr, nullptr);
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

GLFWwindow* Window::GetGLFWHandle() const
{
	return window;
}
