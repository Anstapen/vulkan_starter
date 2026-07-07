#include "VKManager.h"
#include "VulkanQueue.h"
#include "VulkanTypeConversions.h"
#include "glfw/glfw3.h"
#include <cstdint>
#include <fstream>

using namespace Backend;

static Mupfel::Logger::SafeLoggerPtr validation_layer_logger;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT				messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*										pUserData);

bool							   VKManager::is_initialized = false;
std::unique_ptr<vk::raii::Context> VKManager::vk_context;
Mupfel::Logger::SafeLoggerPtr	   VKManager::logger;

void VKManager::Init()
{
	if (is_initialized)
	{
		return;
	}

	glfwInit();

	logger = Mupfel::Logger::Create("VKManager");
	is_initialized = true;
	vk_context = std::make_unique<vk::raii::Context>();
}

void Backend::VKManager::Shutdown() { glfwTerminate(); }

VulkanContext VKManager::CreateVulkanContext(
	const Window&								window,
	const std::vector<VKQueueFamilyProperties>& wanted_queues,
	const std::vector<const char*>&				wanted_extensions,
	const std::vector<const char*>&				wanted_validation_layers)
{
	assert(is_initialized && "VKManager::Init() must be called before creating a Vulkan context!");
	vk::raii::Instance instance = CreateInstance(wanted_extensions, wanted_validation_layers);

	vk::raii::SurfaceKHR surface = CreateSurface(instance, window);

	vk::raii::PhysicalDevice phys_device = SelectBestDevice(instance, surface);

	VulkanQueues	 actual_queues;
	vk::raii::Device device =
		CreateLogicalDevice(phys_device, actual_queues, wanted_queues, {vk::KHRSwapchainExtensionName}, surface);

	std::vector<VulkanCommandPool> command_pools;
	CreateCommandPools(device, phys_device, wanted_queues, command_pools);

	return VulkanContext(
		std::move(instance), std::move(phys_device), std::move(device), std::move(actual_queues), std::move(surface),
		std::move(command_pools));
}

VulkanSwapChain
VKManager::CreateSwapChain(const VulkanContext& context, const Window& window, uint32_t frames_in_flight)
{
	auto surfaceCapabilities = context.phys_device.getSurfaceCapabilitiesKHR(*context.surface);
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = context.phys_device.getSurfaceFormatsKHR(*context.surface);
	std::vector<vk::PresentModeKHR>	  presentModes = context.phys_device.getSurfacePresentModesKHR(*context.surface);

	vk::Extent2D swapChainExtent = SelectSwapExtent(surfaceCapabilities, window);
	uint32_t	 minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

	vk::SurfaceFormatKHR swapChainSurfaceFormat = SelectSurfaceFormat(surfaceFormats);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.surface = *context.surface,
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
		.clipped = true};

	auto swap_chain = vk::raii::SwapchainKHR(context.device, swapChainCreateInfo);

	return VulkanSwapChain(
		context.device, std::move(swap_chain), swapChainSurfaceFormat, swapChainExtent, frames_in_flight);
}

VulkanPipeline Backend::VKManager::CreatePipeline(
	const VulkanContext&			   context,
	const Ping::PipelineSpecification& specification,
	const VulkanSwapChain&			   swapchain)
{
	logger->info("Creating pipeline with shader file: {}", specification.shaderFilePath);
	std::vector<char>		   shaderCode = readFile(specification.shaderFilePath);
	vk::ShaderModuleCreateInfo createInfo{
		.codeSize = shaderCode.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())};
	vk::raii::ShaderModule			  shaderModule{context.device, createInfo};
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"};
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
	vk::PipelineShaderStageCreateInfo  shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
	std::vector<vk::DynamicState>	   dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	vk::PipelineDynamicStateCreateInfo dynamicState{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};

	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

	for (const auto& attr : specification.vertexLayout.attributes)
	{
		attributeDescriptions.push_back(
			{.location = attr.location,
			 .binding = specification.vertexLayout.binding,
			 .format = ToVulkan(attr.format),
			 .offset = attr.offset});
	}

	vk::VertexInputBindingDescription bindingDescription = {
		.binding = specification.vertexLayout.binding,
		.stride = specification.vertexLayout.stride,
		.inputRate = specification.vertexLayout.inputRate == Ping::VertexInputRate::Instance
						 ? vk::VertexInputRate::eInstance
						 : vk::VertexInputRate::eVertex};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data()};

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
	vk::Viewport							 viewport{0.0f,
						  0.0f,
						  static_cast<float>(swapchain.swapChainExtent.width),
						  static_cast<float>(swapchain.swapChainExtent.height),
						  0.0f,
						  1.0f};
	vk::PipelineViewportStateCreateInfo		 viewportState{.viewportCount = 1, .scissorCount = 1};
	vk::Rect2D								 scissor{vk::Offset2D{0, 0}, swapchain.swapChainExtent};

	/* Rasterizer */
	vk::PipelineRasterizationStateCreateInfo rasterizer{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.depthBiasEnable = vk::False,
		.lineWidth = 1.0f};
	/* Multisampling */
	vk::PipelineMultisampleStateCreateInfo multisampling{
		.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

	/* color blending */
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = vk::True,
		.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.colorBlendOp = vk::BlendOp::eAdd,
		.srcAlphaBlendFactor = vk::BlendFactor::eOne,
		.dstAlphaBlendFactor = vk::BlendFactor::eZero,
		.alphaBlendOp = vk::BlendOp::eAdd,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
						  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment};

	vk::raii::PipelineLayout pipelineLayout = nullptr;

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0, .pushConstantRangeCount = 0};

	pipelineLayout = vk::raii::PipelineLayout(context.device, pipelineLayoutInfo);

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
		{.stageCount = 2,
		 .pStages = shaderStages,
		 .pVertexInputState = &vertexInputInfo,
		 .pInputAssemblyState = &inputAssembly,
		 .pViewportState = &viewportState,
		 .pRasterizationState = &rasterizer,
		 .pMultisampleState = &multisampling,
		 .pColorBlendState = &colorBlending,
		 .pDynamicState = &dynamicState,
		 .layout = pipelineLayout,
		 .renderPass = nullptr},
		{.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapchain.swapChainSurfaceFormat.format}};

	auto graphicsPipeline =
		vk::raii::Pipeline(context.device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

	return VulkanPipeline(std::move(graphicsPipeline), std::move(shaderModule), std::move(pipelineLayout));
}

VulkanCommandBuffers
Backend::VKManager::CreateCommandBuffers(const VulkanContext& context, Ping::QueueType type, uint32_t num_buffers)
{
	vk::raii::CommandBuffers cmd_buffers(nullptr);

	for (const auto& pool : context.command_pools)
	{
		if (pool.type == type)
		{
			vk::CommandBufferAllocateInfo allocInfo{
				.commandPool = pool.commandPool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = num_buffers};

			cmd_buffers = std::move(vk::raii::CommandBuffers(context.device, allocInfo));
		}
	}

	VulkanCommandBuffers vk_buffers;

	for (auto& buf : cmd_buffers)
	{
		vk_buffers.emplace_back(VulkanCommandBuffer(
			std::move(buf), vk::raii::Fence(context.device, {.flags = vk::FenceCreateFlagBits::eSignaled})));
	}

	return vk_buffers;
}

VulkanBuffer Backend::VKManager::CreateBuffer(
	const VulkanContext& context,
	size_t				 size,
	Ping::BufferUsage	 usage,
	Ping::MemoryProperty property)
{
	vk::BufferCreateInfo bufferInfo{
		.size = static_cast<vk::DeviceSize>(size),
		.usage = ToVulkan(usage),
		.sharingMode = vk::SharingMode::eExclusive};

	auto buffer = vk::raii::Buffer(context.device, bufferInfo);

	VkMemoryRequirements memRequirements = buffer.getMemoryRequirements();

	vk::MemoryAllocateInfo memoryAllocateInfo{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(context.phys_device, memRequirements.memoryTypeBits, property)};

	auto buffer_memory = vk::raii::DeviceMemory(context.device, memoryAllocateInfo);

	buffer.bindMemory(*buffer_memory, 0);

	/* If the memory properties indicate host-mapped memory, map it now */
	void* mapped_memory = nullptr;

	if (Ping::HasFlag(property, Ping::MemoryProperty::HostVisible))
	{
		mapped_memory = buffer_memory.mapMemory(0, bufferInfo.size);
	}

	return VulkanBuffer(std::move(buffer), std::move(buffer_memory), mapped_memory, memRequirements.size);
}

void Backend::VKManager::FlushMappedMemoryRanges(const VulkanContext& context, const VulkanBuffer& buffer)
{
	vk::MappedMemoryRange range{.memory = buffer.memory, .offset = 0, .size = buffer.size};
	context.device.flushMappedMemoryRanges({range});
	// context.device.invalidateMappedMemoryRanges({range});
}

void Backend::VKManager::transitionImageLayout(
	VulkanCommandBuffer&			   cmd_buffer,
	VulkanSwapChain&				   swapchain,
	uint32_t						   imageIndex,
	const Ping::ImageLayoutTransition& layout_transition)
{
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = ToVulkan(layout_transition.srcStage),
		.srcAccessMask = ToVulkan(layout_transition.srcAccessMask),
		.dstStageMask = ToVulkan(layout_transition.dstStage),
		.dstAccessMask = ToVulkan(layout_transition.dstAccessMask),
		.oldLayout = ToVulkan(layout_transition.oldLayout),
		.newLayout = ToVulkan(layout_transition.newLayout),
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = swapchain.swapChainImages[imageIndex],
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1}};
	vk::DependencyInfo dependency_info = {
		.dependencyFlags = {}, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier};
	cmd_buffer.commandBuffer.pipelineBarrier2(dependency_info);
}

void Backend::VKManager::beginRendering(
	VulkanCommandBuffer& cmd_buffer,
	VulkanSwapChain&	 swapchain,
	uint32_t			 imageIndex)
{
	vk::ClearValue				clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = swapchain.swapChainImageViews[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor};

	vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0, 0}, .extent = swapchain.swapChainExtent},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo};

	cmd_buffer.commandBuffer.beginRendering(renderingInfo);

	cmd_buffer.commandBuffer.setViewport(
		0, vk::Viewport(
			   0.0f, 0.0f, static_cast<float>(swapchain.swapChainExtent.width),
			   static_cast<float>(swapchain.swapChainExtent.height), 0.0f, 1.0f));
	cmd_buffer.commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.swapChainExtent));
}

uint32_t Backend::VKManager::GetQueueIndex(const VulkanContext& context, Ping::QueueType wanted_queue_type)
{
	for (uint32_t i = 0; i < context.queues.size(); i++)
	{
		if (context.queues[i].type == wanted_queue_type)
		{
			return i;
		}
	}

	throw std::runtime_error("The wanted queue could not be found in the given context!");
}

void Backend::VKManager::WaitForCommands(const vk::raii::Device& device) { device.waitIdle(); }

vk::raii::Instance VKManager::CreateInstance(
	const std::vector<const char*>& wanted_extensions,
	const std::vector<const char*>& wanted_validation_layers)
{
	constexpr vk::ApplicationInfo app_info{
		.pApplicationName = "Vulkan Playground",
		.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
		.apiVersion = VK_API_VERSION_1_0};

	vk::InstanceCreateInfo instance_create_info{.pApplicationInfo = &app_info};

	VKExtensions extensions;

#ifndef NDEBUG
	VKValidationLayers validation_layers;
	for (auto& e : wanted_validation_layers)
	{
		if (VKValidationLayers::IsValidationLayerSupported(e))
		{
			validation_layers.Add(e);
		}
		else
		{
			if (strlen(e) > 0)
			{
				logger->warn("validation layer {} is not supported!", e);
			}
		}
	}

	if (validation_layers.Size() > 0)
	{
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.Size());
		instance_create_info.ppEnabledLayerNames = validation_layers.Data();
	}

	/*
	 * Additionally to the validation layers, we also enable the
	 * VK_EXT_DEBUG_UTILS_EXTENSION_NAME extension.
	 */
	if (VKExtensions::IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		extensions.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		logger->warn("Debug mode enabled, but {} is not supported!", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
#endif

	for (auto& e : wanted_extensions)
	{
		if (VKExtensions::IsExtensionSupported(e))
		{
			extensions.Add(e);
		}
		else
		{
			if (strlen(e) > 0)
			{
				logger->warn("extension {} is not supported!", e);
			}
		}
	}

	AddRequiredExtensions(extensions);

	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.Size());

	if (instance_create_info.enabledExtensionCount > 0)
	{
		instance_create_info.ppEnabledExtensionNames = extensions.Data();
	}

	vk::raii::Instance instance(*vk_context, instance_create_info);

	if (!validation_layer_logger)
	{
		validation_layer_logger = Mupfel::Logger::Create("ValidationLayer");
		SetupDebugCallback(instance, debugCallback);
	}

	return instance;
}

vk::raii::SurfaceKHR Backend::VKManager::CreateSurface(vk::raii::Instance& instance, const Window& window)
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, window.GetGLFWHandle(), nullptr, &_surface) != 0)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	return vk::raii::SurfaceKHR(instance, _surface);
}

void VKManager::AddRequiredExtensions(VKExtensions& extensions)
{
	uint32_t	 glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		extensions.Add(glfwExtensions[i]);
	}
}

void Backend::VKManager::CreateCommandPools(
	const vk::raii::Device&						device,
	const vk::raii::PhysicalDevice				phys_device,
	const std::vector<VKQueueFamilyProperties>& queues,
	std::vector<VulkanCommandPool>&				command_pools)
{
	for (const auto& queue : queues)
	{
		/* Get the queue family index */
		auto queue_family_index = queue.GetQueueIndexFromPhysicalDevice(phys_device);
		if (!queue_family_index.has_value())
		{
			logger->error("Unable to retrieve queue family index from physical device!");
			throw std::runtime_error("Unable to retrieve queue family index from physical device!");
		}
		vk::CommandPoolCreateInfo pool_info{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = queue_family_index.value()};
		vk::raii::CommandPool command_pool(device, pool_info);

		Ping::QueueType wanted_type = Ping::QueueType::Compute;
		/* very verbose for now */
		if (queue.wanted_flags & vk::QueueFlagBits::eGraphics)
		{
			wanted_type = Ping::QueueType::Graphics;
		}
		else if (queue.wanted_flags & vk::QueueFlagBits::eTransfer)
		{
			wanted_type = Ping::QueueType::Transfer;
		}

		command_pools.emplace_back(VulkanCommandPool(wanted_type, std::move(command_pool)));
	}
}

vk::raii::PhysicalDevice VKManager::SelectBestDevice(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface)
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

	if (suitable_device_index == phys_devices.size())
	{
		throw std::runtime_error("failed to find suitable GPU!");
	}

	return phys_devices[suitable_device_index];
}

vk::raii::DebugUtilsMessengerEXT
VKManager::SetupDebugCallback(vk::raii::Instance& instance, PFN_vkDebugUtilsMessengerCallbackEXT user_callback)
{
#ifdef NDEBUG
	return vk::raii::DebugMessenger(nullptr);
#else

	vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info;

	messenger_create_info.messageSeverity =
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	messenger_create_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
										vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
										vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
	messenger_create_info.setPfnUserCallback(user_callback);
	return instance.createDebugUtilsMessengerEXT(messenger_create_info);
#endif
}

bool VKManager::IsDeviceSuitable(const vk::raii::PhysicalDevice& device)
{
	std::vector<vk::QueueFamilyProperties> queue_family_properties = device.getQueueFamilyProperties();

	if (queue_family_properties.empty())
	{
		return false;
	}

	for (const auto& q : queue_family_properties)
	{
		if (q.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			return true;
		}
	}

	return false;
}

vk::raii::Device VKManager::CreateLogicalDevice(
	const vk::raii::PhysicalDevice&				phys_device,
	VulkanQueues&								queues,
	const std::vector<VKQueueFamilyProperties>& wanted_queues,
	const std::vector<const char*>&				wanted_extensions,
	vk::raii::SurfaceKHR&						surface)
{
	(void)wanted_extensions;
	/* First we need to prepare the QueueCreateInfo structures */
	/* The queue priority values need to be pointers to float */
	std::vector<float>					   queue_priorities(wanted_queues.size(), 1.0f);
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos(wanted_queues.size());

	for (uint32_t i = 0; i < wanted_queues.size(); i++)
	{
		std::optional<uint32_t> q_index = wanted_queues[i].GetQueueIndexFromPhysicalDevice(phys_device);
		auto&					current_queue_create_info = queue_create_infos[i];
		if (!q_index.has_value())
		{
			throw std::runtime_error("Device has no suitable queues!");
		}

		/*
			If the queue to be created is a graphics queue,
			we need to check the presentation suport.
			This is lazy for now (there could be multiple graphics queue,
			but we only take the first one and check presentation support.
		*/
		if ((wanted_queues[i].wanted_flags & vk::QueueFlagBits::eGraphics) &&
			!wanted_queues[i].CheckSurfaceSupport(phys_device, surface))
		{
			throw std::runtime_error("The graphics queue of the device has no presentation "
									 "support!");
		}

		current_queue_create_info.pQueuePriorities = &queue_priorities[i];
		current_queue_create_info.queueCount = wanted_queues[i].wanted_queue_instances;
		current_queue_create_info.queueFamilyIndex = q_index.value();

		if (i != 0 && i < (wanted_queues.size() - 1))
		{
			current_queue_create_info.pNext = &queue_create_infos[i + 1];
		}
	}

	/* Select device features */
	vk::StructureChain<
		vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{},
			{.shaderDrawParameters = true},
			{.synchronization2 = true, .dynamicRendering = true},
			{.extendedDynamicState = true}};

	vk::DeviceCreateInfo device_create_info;

	device_create_info.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(wanted_extensions.size());
	device_create_info.ppEnabledExtensionNames = wanted_extensions.data();
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());

	vk::raii::Device device = vk::raii::Device(phys_device, device_create_info);

	for (uint32_t i = 0; i < wanted_queues.size(); i++)
	{
		Ping::QueueType q_type = Ping::QueueType::Graphics;
		vk::raii::Queue queue(device, queue_create_infos[i].queueFamilyIndex, 0);
		if (wanted_queues[i].wanted_flags & vk::QueueFlagBits::eCompute)
		{
			q_type = Ping::QueueType::Compute;
		}
		else if (wanted_queues[i].wanted_flags & vk::QueueFlagBits::eTransfer)
		{
			q_type = Ping::QueueType::Transfer;
		}
		queues.push_back(VulkanQueue(q_type, std::move(queue)));
	}

	return device;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT				messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*										pUserData)
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
		[](const auto& format)
		{
			return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR Backend::VKManager::SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	assert(std::ranges::any_of(
		availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(
			   availablePresentModes,
			   [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; })
			   ? vk::PresentModeKHR::eMailbox
			   : vk::PresentModeKHR::eFifo;
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
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
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

std::vector<char> Backend::VKManager::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}
	std::vector<char> buffer(file.tellg());

	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

	file.close();

	return buffer;
}

uint32_t Backend::VKManager::findMemoryType(
	const vk::raii::PhysicalDevice& phys_devicee,
	uint32_t						type_filter,
	Ping::MemoryProperty			property)
{
	vk::PhysicalDeviceMemoryProperties memProperties = phys_devicee.getMemoryProperties();

	vk::MemoryPropertyFlags vk_properties = ToVulkan(property);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & vk_properties) == vk_properties)
		{
			return i;
		}
	}

	/* No memory type was found */
	throw std::runtime_error("Unable to find fitting memory type!");
}
