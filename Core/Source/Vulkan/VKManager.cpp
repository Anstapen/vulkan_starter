#include "VKManager.h"
#include <cstdint>

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

	glfwInit();

	logger = Mupfel::Logger::Create("VKManager");
	is_initialized = true;
	context = std::make_unique<vk::raii::Context>();
}

void Backend::VKManager::Shutdown()
{
	glfwTerminate();
}

VulkanContext VKManager::CreateVulkanContext(const Window &window,
	const std::vector<VKQueueFamilyProperties>& wanted_queues,
	const std::vector<const char*>& wanted_extensions,
	const std::vector<const char*>& wanted_validation_layers)
{
	assert(is_initialized && "VKManager::Init() must be called before creating a Vulkan context!");
	vk::raii::Instance instance = CreateInstance(wanted_extensions, wanted_validation_layers);

	vk::raii::SurfaceKHR surface = CreateSurface(instance, window);

	vk::raii::PhysicalDevice phys_device = SelectBestDevice(instance, surface);

	std::vector<vk::raii::Queue> actual_queues;
	vk::raii::Device device = CreateLogicalDevice(phys_device, actual_queues, wanted_queues, { vk::KHRSwapchainExtensionName }, surface);

	return VulkanContext(std::move(instance), std::move(phys_device), std::move(device), actual_queues, std::move(surface));
}

VulkanSwapChain VKManager::CreateSwapChain(
	const VulkanContext& context,
	const Window& window)
{
	auto surfaceCapabilities = context.phys_device.getSurfaceCapabilitiesKHR(*context.surface);
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = context.phys_device.getSurfaceFormatsKHR(*context.surface);
	std::vector<vk::PresentModeKHR> presentModes = context.phys_device.getSurfacePresentModesKHR(*context.surface);

	vk::Extent2D swapChainExtent = SelectSwapExtent(surfaceCapabilities, window);
	uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

	vk::SurfaceFormatKHR swapChainSurfaceFormat = SelectSurfaceFormat(surfaceFormats);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{ .surface = *context.surface,
											   .minImageCount = minImageCount,
											   .imageFormat = swapChainSurfaceFormat.format,
											   .imageColorSpace = swapChainSurfaceFormat.colorSpace,
											   .imageExtent = swapChainExtent,
											   .imageArrayLayers = 1,
											   .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
											   .imageSharingMode = vk::SharingMode::eExclusive,
											   .preTransform = surfaceCapabilities.currentTransform,
											   .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
											   .presentMode = SelectPresentMode(presentModes),
											   .clipped = true };

	auto swap_chain = vk::raii::SwapchainKHR(context.device, swapChainCreateInfo);

	return VulkanSwapChain(std::move(swap_chain), swapChainSurfaceFormat, swapChainExtent);
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

vk::raii::SurfaceKHR Backend::VKManager::CreateSurface(vk::raii::Instance& instance, const Window& window)
{
	VkSurfaceKHR       _surface;
	if (glfwCreateWindowSurface(*instance, window.GetGLFWHandle(), nullptr, &_surface) != 0) {
		throw std::runtime_error("failed to create window surface!");
	}
	return vk::raii::SurfaceKHR(instance, _surface);
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

vk::raii::PhysicalDevice VKManager::SelectBestDevice(vk::raii::Instance& instance,
	vk::raii::SurfaceKHR& surface)
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

	return phys_devices[suitable_device_index];
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

vk::raii::Device VKManager::CreateLogicalDevice(const vk::raii::PhysicalDevice& phys_device,
	std::vector<vk::raii::Queue>& queues,
	const std::vector<VKQueueFamilyProperties>& wanted_queues,
	const std::vector<const char*>& wanted_extensions,
	vk::raii::SurfaceKHR& surface)
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

		/*
			If the queue to be created is a graphics queue,
			we need to check the presentation suport.
			This is lazy for now (there could be multiple graphics queue,
			but we only take the first one and check presentation support.
		*/
		if ((wanted_queues[i].wanted_flags & vk::QueueFlagBits::eGraphics) && !wanted_queues[i].CheckSurfaceSupport(phys_device, surface))
		{
 			throw std::runtime_error("The graphics queue of the device has no presentation support!");
		}

		current_queue_create_info.pQueuePriorities = &queue_priorities[i];
		current_queue_create_info.queueCount = wanted_queues[i]
			.wanted_queue_instances;
		current_queue_create_info.queueFamilyIndex = q_index.value();

		if (i != 0 && i < (wanted_queues.size() - 1)) {
			current_queue_create_info.pNext = &queue_create_infos[i + 1];
		}
	}

	/* Select device features */
	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{},
			{.shaderDrawParameters = true},
			{.dynamicRendering = true},
			{.extendedDynamicState = true}
	};


	vk::DeviceCreateInfo device_create_info;

	device_create_info.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(wanted_extensions.size());
	device_create_info.ppEnabledExtensionNames = wanted_extensions.data()
		;
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());


	vk::raii::Device device = vk::raii::Device(phys_device, device_create_info);

	/* retieve the queue handles */
	vk::raii::Queue graphicsQueue(device, queue_create_infos[0].queueFamilyIndex, 0);

	for (const auto& q : queue_create_infos)
	{
		vk::raii::Queue queue(device, q.queueFamilyIndex, 0);
		queues.push_back(std::move(queue));
	}

	return device;
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

vk::SurfaceFormatKHR Backend::VKManager::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	const auto formatIt = std::ranges::find_if(
		availableFormats,
		[](const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR  Backend::VKManager::SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(availablePresentModes,
		[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
		vk::PresentModeKHR::eMailbox :
		vk::PresentModeKHR::eFifo;
}

vk::Extent2D Backend::VKManager::SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	int width, height;
	glfwGetFramebufferSize(window.GetGLFWHandle(), &width, &height);

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

uint32_t Backend::VKManager::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
{
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
	if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = surfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}