#include "Renderer.h"
#include <iostream>
#include <cassert>
#include <ranges>

using namespace Mupfel;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

void Renderer::Init()
{
	std::cout << "Renderer::Init" << std::endl;
	InitWindow();
	InitVulkan();
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

void Mupfel::Renderer::InitVulkan()
{
	createInstance();
}

void Mupfel::Renderer::createInstance()
{
	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "My first App",
		.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14 };

	vk::InstanceCreateInfo createInfo{
		.pApplicationInfo = &appInfo
	};

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	auto extensionProperties = context.enumerateInstanceExtensionProperties();

	/* Check if required GLFW extensions are supported by the vulkan implementation */
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		if (std::ranges::none_of(extensionProperties,
			[glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
			{ return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
		{
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
		}
	}

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	// Get the required layers
	std::vector<char const*> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	// Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties = context.enumerateInstanceLayerProperties();
	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
		[&layerProperties](auto const& requiredLayer) {
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
		});
	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
	}
	
	createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	createInfo.ppEnabledLayerNames = requiredLayers.data();

	instance = vk::raii::Instance(context, createInfo);
}
