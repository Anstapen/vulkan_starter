#include "VKManager.h"
#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace Mupfel;

Logger::SafeLoggerPtr VKManager::logger;
VkDebugUtilsMessengerEXT VKManager::debugMessenger;

void VKManager::Init()
{
	logger = Logger::Create("VKManager");
}

VkInstance VKManager::CreateInstance()
{
	return CreateInstance({ "" }, { "" });
}

VkInstance VKManager::CreateInstance(
	const std::vector<const char*>& wanted_extensions,
	const std::vector<const char*>& wanted_validation_layers)
{
	VkApplicationInfo app_info;
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = "Vulkan Playground";
	app_info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_create_info;
	memset(&instance_create_info, 0, sizeof(VkInstanceCreateInfo));
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &app_info;

	VKExtensions extensions;

#ifndef NDEBUG
	VKValidationLayers validation_layers;
	for (auto& e : wanted_validation_layers) {
		if (VKValidationLayers::IsValidationLayerSupported(e)) {
			validation_layers.Add(e);
		}
		else {
			if (strlen(e) > 0) {
				logger->warn("validation layer {} is not supported!", e);
			}
		}
	}

	if (validation_layers.Size() > 0) {
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.Size());
		instance_create_info.ppEnabledLayerNames = validation_layers.Data();
		logger->info("Using the following validation layers:");
		validation_layers.Print(logger);
	}

	/*
	 * Additionally to the validation layers, we also enable the
	 * VK_EXT_DEBUG_UTILS_EXTENSION_NAME extension.
	 */
	if (VKExtensions::IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
		extensions.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else {
		logger->warn("Debug mode enabled, but {} is not supported!",
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
#endif

	for (auto& e : wanted_extensions) {
		if (VKExtensions::IsExtensionSupported(e)) {
			extensions.Add(e);
		}
		else {
			if (strlen(e) > 0) {
				logger->warn("extension {} is not supported!", e);
			}
		}
	}

	AddRequiredExtensions(extensions);

	logger->info("Using the following extensions:");
	extensions.Print(logger);

	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.Size());

	if (instance_create_info.enabledExtensionCount > 0) {
		instance_create_info.ppEnabledExtensionNames = extensions.Data();
	}

	VkInstance instance;
	VkResult result = vkCreateInstance(&instance_create_info, nullptr,
		&instance);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	return instance;
}

void VKManager::DestroyInstance(
	VkInstance instance)
{
	if (instance != nullptr) {
		vkDestroyInstance(instance, nullptr);
	}
}

void VKManager::AddRequiredExtensions(
	VKExtensions& extensions)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; i++) {
		extensions.Add(glfwExtensions[i]);
	}
}

VkDevice VKManager::SelectBestDevice(
	VkInstance instance,
	const std::vector<VKQueueFamilyProperties>& wanted_queues)
{
	uint32_t count = 0;
	VkResult result = vkEnumeratePhysicalDevices(instance, &count, NULL);

	if (result != VK_SUCCESS) {
		logger->error("Could not retrieve number of physical devices!");
	}

	logger->info("Found {} physical devices.", count);

	/* Create array for the physical devices */
	std::vector<VkPhysicalDevice> devices(count);
	result = vkEnumeratePhysicalDevices(instance, &count, devices.data());

	for (const auto& d : devices)
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(d, &device_properties);
		logger->info(device_properties.deviceName);
	}

	/* For now, we just take the first device that is suitable */
	VkPhysicalDevice suitable_device = nullptr;
	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			suitable_device = device;
		}
	}

	if (suitable_device == nullptr) {
		throw std::runtime_error("failed to find suitable GPU!");
	}

	return CreateLogicalDevice(suitable_device, wanted_queues, {});
}

void Mupfel::VKManager::CleanUpDevice(VkDevice device)
{
	vkDestroyDevice(device, nullptr);
}

void VKManager::SetupDebugCallback(
	VkInstance instance,
	PFN_vkDebugUtilsMessengerCallbackEXT user_callback)
{
#ifdef NDEBUG
	return;
#else

	VkDebugUtilsMessengerCreateInfoEXT messenger_create_info;
	memset(&messenger_create_info, 0,
		sizeof(VkDebugUtilsMessengerCreateInfoEXT));
	messenger_create_info
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messenger_create_info
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messenger_create_info
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messenger_create_info.pfnUserCallback = user_callback;
	if (CreateDebugUtilsMessengerEXT(instance, &messenger_create_info, nullptr,
		&debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
#endif
}

VkResult VKManager::CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VKManager::DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT in_debug_messenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, in_debug_messenger, pAllocator);
	}
}

bool VKManager::IsDeviceSuitable(
	VkPhysicalDevice device)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
		nullptr);

	if (queue_family_count == 0) {
		return false;
	}

	std::vector<VkQueueFamilyProperties> queue_family_properties(
		queue_family_count);

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
		queue_family_properties.data());

	for (const auto& q : queue_family_properties) {
		if (q.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			return true;
		}
	}

	return false;
}

VkDevice VKManager::CreateLogicalDevice(
	VkPhysicalDevice phys_device,
	const std::vector<VKQueueFamilyProperties>& wanted_queues,
	const std::vector<const char*>& wanted_extensions)
{
	(void)wanted_extensions;
	/* First we need to prepare the QueueCreateInfo structures */
	/* The queue priority values need to be pointers to float */
	std::vector<float> queue_priorities(wanted_queues.size(), 1.0f);
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos(
		wanted_queues.size());

	for (uint32_t i = 0; i < wanted_queues.size(); i++) {
		std::optional<uint32_t> q_index = wanted_queues[i]
			.GetQueueIndexFromPhysicalDevice(
				phys_device);
		auto& current_queue_create_info = queue_create_infos[i];
		if (!q_index.has_value()) {
			throw std::runtime_error("Device has no suitable queues!");
		}

		current_queue_create_info
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		current_queue_create_info.flags = 0;
		current_queue_create_info.pQueuePriorities = &queue_priorities[i];
		current_queue_create_info.queueCount = wanted_queues[i]
			.wanted_queue_instances;
		current_queue_create_info.queueFamilyIndex = q_index.value();

		if (i != 0 && i < (wanted_queues.size() - 1)) {
			current_queue_create_info.pNext = &queue_create_infos[i + 1];
		}
	}

	VkPhysicalDeviceFeatures device_features{};
	(void)device_features;

	VkDeviceCreateInfo device_create_info;
	memset(&device_create_info, 0, sizeof(VkDeviceCreateInfo));

	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	VkDevice device;
	if (vkCreateDevice(phys_device, &device_create_info, nullptr, &device) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(phys_device, &device_properties);

	logger->info("Created logical device of {}", device_properties.deviceName);

	return device;
}
