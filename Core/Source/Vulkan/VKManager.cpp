#define GLFW_INCLUDE_VULKAN
#include "VKManager.h"
#include <cstdint>
#include <GLFW/glfw3.h>

using namespace Backend;

static Mupfel::Logger::SafeLoggerPtr validation_layer_logger;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

bool VKManager::is_initialized = false;
std::unique_ptr<vk::raii::Context> VKManager::context;
Mupfel::Logger::SafeLoggerPtr VKManager::logger;

void VKManager::Init()
{
	if (is_initialized)
	{
		return;
	}

	logger = Mupfel::Logger::Create("VKManager");
	is_initialized = true;
	context = std::make_unique<vk::raii::Context>();
}

VulkanContext VKManager::CreateVulkanContext(const std::vector<VKQueueFamilyProperties>& wanted_queues, const std::vector<const char*>& wanted_extensions, const std::vector<const char*>& wanted_validation_layers)
{
	(void)wanted_queues;
	assert(is_initialized && "VKManager::Init() must be called before creating a Vulkan context!");
	vk::raii::Instance instance = CreateInstance(wanted_extensions, wanted_validation_layers);
	vk::raii::Device device = SelectBestDevice(instance, wanted_queues);
	vk::raii::DebugUtilsMessengerEXT dbg = SetupDebugCallback(instance, debugCallback);

	return VulkanContext(std::move(instance), std::move(device), std::move(dbg));
}

vk::raii::Instance
VKManager::CreateInstance(const std::vector<const char*>& wanted_extensions,
	const std::vector<const char*>& wanted_validation_layers)
{
	constexpr vk::ApplicationInfo app_info{
		.pApplicationName = "Vulkan Playground",
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};


	vk::InstanceCreateInfo instance_create_info{
		.pApplicationInfo = &app_info
	};

	instance_create_info.flags = vk::InstanceCreateFlags(123);

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

	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.Size());

	if (instance_create_info.enabledExtensionCount > 0) {
		instance_create_info.ppEnabledExtensionNames = extensions.Data();
	}
	
	vk::raii::Instance instance(*context, instance_create_info);

	if (!validation_layer_logger)
	{
		validation_layer_logger = Mupfel::Logger::Create("ValidationLayer");
		SetupDebugCallback(instance, debugCallback);
	}

	return instance;
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

vk::raii::Device VKManager::SelectBestDevice(
	vk::raii::Instance &instance,
	const std::vector<VKQueueFamilyProperties>& wanted_queues)
{
	std::vector<vk::raii::PhysicalDevice> phys_devices = instance.enumeratePhysicalDevices();

	logger->info("Found {} physical devices.", phys_devices.size());

	for (const auto& d : phys_devices)
	{
		VkPhysicalDeviceProperties device_properties = d.getProperties();
		logger->info(device_properties.deviceName);
	}

	uint32_t suitable_device_index = static_cast<uint32_t>(phys_devices.size());
	for (uint32_t i = 0; i < phys_devices.size(); i++)
	{
		if (IsDeviceSuitable(phys_devices[i]))
		{
			suitable_device_index = i;
		}
	}

	if (suitable_device_index == phys_devices.size()) {
		throw std::runtime_error("failed to find suitable GPU!");
	}

	return CreateLogicalDevice(phys_devices[suitable_device_index], wanted_queues, {});
}

vk::raii::DebugUtilsMessengerEXT VKManager::SetupDebugCallback(
	vk::raii::Instance& instance,
	PFN_vkDebugUtilsMessengerCallbackEXT user_callback)
{
#ifdef NDEBUG
	return vk::raii::DebugMessenger(nullptr);
#else
	
	vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info;

	messenger_create_info
		.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	messenger_create_info
		.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
	messenger_create_info.setPfnUserCallback(user_callback);
	return instance.createDebugUtilsMessengerEXT(messenger_create_info);
#endif
}

bool VKManager::IsDeviceSuitable(
	const vk::raii::PhysicalDevice& device)
{
	std::vector<vk::QueueFamilyProperties> queue_family_properties = device.getQueueFamilyProperties();

	if (queue_family_properties.empty()) {
		return false;
	}

	for (const auto& q : queue_family_properties) {
		if (q.queueFlags & vk::QueueFlagBits::eGraphics) {
			return true;
		}
	}

	return false;
}

vk::raii::Device VKManager::CreateLogicalDevice(
	const vk::raii::PhysicalDevice &phys_device,
	const std::vector<VKQueueFamilyProperties>& wanted_queues,
	const std::vector<const char*>& wanted_extensions)
{
	(void)wanted_extensions;
	/* First we need to prepare the QueueCreateInfo structures */
	/* The queue priority values need to be pointers to float */
	std::vector<float> queue_priorities(wanted_queues.size(), 1.0f);
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos(
		wanted_queues.size());

	for (uint32_t i = 0; i < wanted_queues.size(); i++) {
		std::optional<uint32_t> q_index = wanted_queues[i]
			.GetQueueIndexFromPhysicalDevice(
				phys_device);
		auto& current_queue_create_info = queue_create_infos[i];
		if (!q_index.has_value()) {
			throw std::runtime_error("Device has no suitable queues!");
		}

		current_queue_create_info.pQueuePriorities = &queue_priorities[i];
		current_queue_create_info.queueCount = wanted_queues[i]
			.wanted_queue_instances;
		current_queue_create_info.queueFamilyIndex = q_index.value();

		if (i != 0 && i < (wanted_queues.size() - 1)) {
			current_queue_create_info.pNext = &queue_create_infos[i + 1];
		}
	}

	vk::PhysicalDeviceFeatures device_features{};
	(void)device_features;

	vk::DeviceCreateInfo device_create_info;
;
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());


	return vk::raii::Device(phys_device, device_create_info);
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	(void)messageSeverity;
	(void)messageTypes;
	(void)pUserData;
	validation_layer_logger->warn(pCallbackData->pMessage);
	return VK_FALSE;
}