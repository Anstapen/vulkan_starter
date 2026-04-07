#include "Renderer.h"
#include <iostream>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <cassert>

using namespace Mupfel;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

void Renderer::Init()
{
	std::cout << "Renderer::Init" << std::endl;
	InitWindow();
}

void Renderer::Render()
{
	assert(window != nullptr);

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void Renderer::DeInit()
{
	if(window != nullptr)
	{
		return;
	}

	glfwDestroyWindow(window);

	glfwTerminate();
}

void Mupfel::Renderer::InitWindow()
{
	glfwInit();
	/* Do not create an OpenGL context */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	/* Make it non-resizable for now */
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	/* Create the window */
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}
