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
	const Window&					   window,
	const std::vector<VKQueueRequest>& wanted_queues,
	const std::vector<const char*>&	   wanted_instance_extensions,
	const std::vector<const char*>&	   wanted_device_extensions,
	const std::vector<const char*>&	   wanted_validation_layers)
{
	assert(is_initialized && "VKManager::Init() must be called before creating a Vulkan context!");
	vk::raii::Instance instance = CreateInstance(wanted_instance_extensions, wanted_validation_layers);

	vk::raii::SurfaceKHR surface = CreateSurface(instance, window);

	vk::raii::PhysicalDevice phys_device = SelectBestDevice(instance, surface);

	std::vector<VKResolvedQueue> resolved_queues = VKQueueFamilyAllocator::Allocate(phys_device, wanted_queues);

	VulkanQueues	 actual_queues;
	vk::raii::Device device =
		CreateLogicalDevice(phys_device, actual_queues, resolved_queues, wanted_device_extensions, surface);

	std::vector<VulkanCommandPool> command_pools;
	CreateCommandPools(device, resolved_queues, command_pools);

	vk::raii::DebugUtilsMessengerEXT debug_messenger = SetupDebugCallback(instance, debugCallback);

	return VulkanContext(
		std::move(instance), std::move(phys_device), std::move(device), std::move(actual_queues), std::move(surface),
		std::move(command_pools), std::move(debug_messenger));
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

vk::raii::DescriptorSetLayout Backend::VKManager::CreateDescriptorSetLayout(
	const VulkanContext&						context,
	const std::vector<Ping::DescriptorBinding>& bindings)
{
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
	layoutBindings.reserve(bindings.size());

	for (const auto& binding : bindings)
	{
		layoutBindings.push_back(
			{.binding = binding.binding,
			 .descriptorType = ToVulkan(binding.type),
			 .descriptorCount = 1,
			 .stageFlags = ToVulkan(binding.stageFlags)});
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo{
		.bindingCount = static_cast<uint32_t>(layoutBindings.size()), .pBindings = layoutBindings.data()};

	return vk::raii::DescriptorSetLayout(context.device, layoutInfo);
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
		.frontFace = vk::FrontFace::eCounterClockwise,
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

	vk::raii::DescriptorSetLayout descriptorSetLayout =
		CreateDescriptorSetLayout(context, specification.descriptorBindings);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
		.setLayoutCount = 1, .pSetLayouts = &*descriptorSetLayout, .pushConstantRangeCount = 0};

	vk::raii::PipelineLayout pipelineLayout(context.device, pipelineLayoutInfo);

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

	return VulkanPipeline(
		std::move(graphicsPipeline), std::move(shaderModule), std::move(descriptorSetLayout),
		std::move(pipelineLayout));
}

VulkanDescriptorPool Backend::VKManager::CreateDescriptorSets(
	const VulkanContext&					context,
	const VulkanPipeline&					pipeline,
	const std::vector<const VulkanBuffer*>& uniform_buffers)
{
	if (uniform_buffers.empty())
	{
		throw std::runtime_error("Tried to create descriptor sets with no uniform buffers!");
	}

	uint32_t set_count = static_cast<uint32_t>(uniform_buffers.size());

	vk::DescriptorPoolSize poolSize{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = set_count};

	vk::DescriptorPoolCreateInfo poolInfo{
		/* Individual sets must be freeable, since ~VulkanDescriptorPool frees them one at a time. */
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = set_count,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize};

	vk::raii::DescriptorPool pool(context.device, poolInfo);

	/* vkAllocateDescriptorSets wants one layout handle per set, even though they're all identical. */
	std::vector<vk::DescriptorSetLayout> layouts(set_count, *pipeline.descriptorSetLayout);
	vk::DescriptorSetAllocateInfo		 allocInfo{
		.descriptorPool = pool, .descriptorSetCount = set_count, .pSetLayouts = layouts.data()};

	vk::raii::DescriptorSets rawSets(context.device, allocInfo);

	for (uint32_t i = 0; i < set_count; i++)
	{
		vk::DescriptorBufferInfo bufferInfo{
			.buffer = *uniform_buffers[i]->buffer, .offset = 0, .range = uniform_buffers[i]->Size()};

		vk::WriteDescriptorSet write{
			.dstSet = rawSets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &bufferInfo};

		context.device.updateDescriptorSets(write, {});
	}

	std::vector<vk::raii::DescriptorSet> sets;
	sets.reserve(set_count);
	for (auto& set : rawSets)
	{
		sets.push_back(std::move(set));
	}

	return VulkanDescriptorPool(std::move(pool), std::move(sets));
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
	/* find queue family indices to share the buffer with */

	vk::BufferCreateInfo bufferInfo{
		.size = static_cast<vk::DeviceSize>(size),
		.usage = ToVulkan(usage),
		.sharingMode = vk::SharingMode::eExclusive};

	const uint32_t queue_indices[2] = {
		context.queues[GetQueueIndex(context, Ping::QueueType::Transfer)].familyIndex,
		context.queues[GetQueueIndex(context, Ping::QueueType::Graphics)].familyIndex};

	if (Ping::HasFlag(property, Ping::MemoryProperty::DeviceLocal))
	{
		/* device local buffer are shared across transfer and graphics queues */
		bufferInfo.sharingMode = vk::SharingMode::eConcurrent;
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = &queue_indices[0];
	}

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
		mapped_memory = buffer_memory.mapMemory(0, VK_WHOLE_SIZE);
	}

	return VulkanBuffer(std::move(buffer), std::move(buffer_memory), mapped_memory, bufferInfo.size, usage, property);
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
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
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

void Backend::VKManager::transitionImageLayout(
	VulkanCommandBuffer&   cmd_buffer,
	const vk::raii::Image& image,
	vk::ImageLayout		   old_layout,
	vk::ImageLayout		   new_layout)
{
	vk::ImageMemoryBarrier2 barrier = {
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = image,
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
		.apiVersion = VK_API_VERSION_1_4};

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
	const vk::raii::Device&				device,
	const std::vector<VKResolvedQueue>& resolved_queues,
	std::vector<VulkanCommandPool>&		command_pools)
{
	for (const auto& r : resolved_queues)
	{
		vk::CommandPoolCreateInfo pool_info{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = r.familyIndex};
		vk::raii::CommandPool command_pool(device, pool_info);
		command_pools.emplace_back(VulkanCommandPool(r.type, std::move(command_pool), r.familyIndex));
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
	const vk::raii::PhysicalDevice&		phys_device,
	VulkanQueues&						queues,
	const std::vector<VKResolvedQueue>& wanted_queues,
	const std::vector<const char*>&		wanted_device_extensions,
	vk::raii::SurfaceKHR&				surface)
{
	std::unordered_map<uint32_t, uint32_t> queueCountPerFamily;
	for (auto& r : wanted_queues)
		queueCountPerFamily[r.familyIndex] = std::max(queueCountPerFamily[r.familyIndex], r.queueIndexInFamily + 1);

	std::vector<std::vector<float>>		   priorities;
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
	for (auto& [familyIndex, count] : queueCountPerFamily)
	{
		priorities.emplace_back(count, 1.0f);
		queue_create_infos.push_back(
			{.queueFamilyIndex = familyIndex, .queueCount = count, .pQueuePriorities = priorities.back().data()});
	}

	auto graphics_it = std::find_if(
		wanted_queues.begin(), wanted_queues.end(),
		[](const VKResolvedQueue& r) { return r.type == Ping::QueueType::Graphics; });
	if (graphics_it != wanted_queues.end() && !phys_device.getSurfaceSupportKHR(graphics_it->familyIndex, *surface))
	{
		throw std::runtime_error("The graphics queue of the device has no presentation support!");
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
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(wanted_device_extensions.size());
	device_create_info.ppEnabledExtensionNames = wanted_device_extensions.data();
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());

	vk::raii::Device device = vk::raii::Device(phys_device, device_create_info);

	for (auto& r : wanted_queues)
	{
		vk::raii::Queue queue(device, r.familyIndex, r.queueIndexInFamily);
		queues.push_back(VulkanQueue(r.type, std::move(queue), r.familyIndex));
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
	assert(
		std::ranges::any_of(
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

void Backend::VKManager::CopyBuffer(
	const VulkanContext& context,
	vk::raii::Buffer&	 srcBuffer,
	vk::raii::Buffer&	 dstBuffer,
	vk::DeviceSize		 size)
{
	VulkanCommandBuffers copy_cmd_buffer = CreateCommandBuffers(context, Ping::QueueType::Transfer, 1);

	if (copy_cmd_buffer.size() != 1)
	{
		throw std::runtime_error("Unable to create Command buffer from transfer queue");
	}

	copy_cmd_buffer[0].Begin(context.device, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	copy_cmd_buffer[0].commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));

	copy_cmd_buffer[0].End();

	uint32_t q_index = VKManager::GetQueueIndex(context, Ping::QueueType::Transfer);

	context.queues[q_index].queue.submit(
		vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*copy_cmd_buffer[0].commandBuffer}, nullptr);
	context.queues[q_index].queue.waitIdle();
}