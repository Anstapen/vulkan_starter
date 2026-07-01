#include "Application.h"
#include <assert.h>
#include "VKManager/VKManager.h"

/* This static logger is used for the vulkan validation layer messages */
Mupfel::Logger::SafeLoggerPtr
Mupfel::Application::validation_layer_logger = nullptr;

Mupfel::Application::Application(
	const std::string& in_name)
	: name(in_name), renderer(), logger(), window(nullptr), instance(nullptr), device(nullptr)
{
}

void Mupfel::Application::Run()
{
	Init();
	MainLoop();
	CleanUp();
}

void Mupfel::Application::Init()
{
	/* Create a Logger for the Application */
	logger = Logger::Create("App");
	logger->info("Initializing");



	/*
	 * If the vulkan logger is not yet initialized (first call to Init),
	 * create a logger for it.
	 */
	if (!validation_layer_logger)
	{
		validation_layer_logger = Logger::Create("VULKAN_VALIDATION");
	}

	VKManager::Init();
	CreateGLFWWindow();
	CreateVulkanInstance();
	VKManager::SetupDebugCallback(instance, debugCallback);
	device = VKManager::SelectBestDevice(instance,
		{
			{VK_QUEUE_GRAPHICS_BIT, 1}
		});
	renderer.Init(instance);
}
void Mupfel::Application::CreateGLFWWindow()
{
	/* Create the GLFW Window context */
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	const std::string window_name("Vulkan Playground");
	uint32_t window_size_x = 800;
	uint32_t window_size_y = 600;
	logger->info("Creating window {} with size {}x{}", window_name,
		window_size_x, window_size_y);
	window = glfwCreateWindow(window_size_x, window_size_y, window_name.c_str(),
		nullptr, nullptr);
	if (window == nullptr) {
		logger->error("Unable to create Window!");
	}
	assert(window);
}

void Mupfel::Application::CreateVulkanInstance()
{
	instance = VKManager::CreateInstance({ "" }, { "VK_LAYER_KHRONOS_validation" });
	logger->info("Received vulkan instance from VKManager: {}",
		(void*)instance);
}

void Mupfel::Application::MainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void Mupfel::Application::CleanUp()
{
	logger->info("Cleaning up Application");

	if (window != nullptr) {
		glfwDestroyWindow(window);
	}

	if (device != nullptr)
	{
		VKManager::CleanUpDevice(device);
		device = nullptr;
	}

	glfwTerminate();
}

VKAPI_ATTR VkBool32 VKAPI_CALL Mupfel::Application::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	(void)messageSeverity;
	(void)messageType;
	(void)pUserData;
	validation_layer_logger->warn(pCallbackData->pMessage);
	return VK_FALSE;
}
