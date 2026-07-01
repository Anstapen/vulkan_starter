#include "Renderer.h"
#include <iostream>
#include <cassert>
#include <ranges>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <filesystem>

using namespace Mupfel;

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	std::vector<char> buffer(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
	file.close();
	return buffer;
}

void Renderer::Init(VkInstance in_instance)
{
	logger = Logger::Create("Renderer");
	logger->info("Init");
	instance = in_instance;
	InitVulkan();
}

void Renderer::Render()
{
	drawFrame();
}

void Renderer::DeInit()
{
}

void Renderer::InitVulkan()
{
#if 0
	createSurface();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createGrahpicsPipeline();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
#endif
}

std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName, "VK_KHR_shader_draw_parameters" };

void Renderer::printQueueFlags(const vk::QueueFamilyProperties& prop)
{
	vk::QueueFlags flags = prop.queueFlags;

	if (flags & vk::QueueFlagBits::eGraphics)
	{
		logger->info("eGraphics");
	}

	if (flags & vk::QueueFlagBits::eCompute)
	{
		logger->info("eCompute");
	}

	if (flags & vk::QueueFlagBits::eDataGraphARM)
	{
		logger->info("eDataGraphARM");
	}

	if (flags & vk::QueueFlagBits::eOpticalFlowNV)
	{
		logger->info("eOpticalFlowNV");
	}

	if (flags & vk::QueueFlagBits::eProtected)
	{
		logger->info("eProtected");
	}

	if (flags & vk::QueueFlagBits::eSparseBinding)
	{
		logger->info("eSparseBinding");
	}

	if (flags & vk::QueueFlagBits::eTransfer)
	{
		logger->info("eTransfer");
	}

	if (flags & vk::QueueFlagBits::eVideoDecodeKHR)
	{
		logger->info("eVideoDecodeKHR");
	}

	if (flags & vk::QueueFlagBits::eVideoEncodeKHR)
	{
		logger->info("eVideoEncodeKHR");
	}
}

bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice const& in_physicalDevice)
{
	vk::PhysicalDeviceProperties prop = in_physicalDevice.getProperties();
	logger->info("Device: {}", prop.deviceName.data());
	logger->info("API version: {}.{}", vk::versionMajor(prop.apiVersion), vk::versionMinor(prop.apiVersion));
	logger->info("Device Type: {}", static_cast<uint32_t>(prop.deviceType));

	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = prop.apiVersion >= vk::ApiVersion13;

	// Check if any of the queue families support graphics operations
	auto queueFamilies = in_physicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		logger->info("Queue {}:", i);
		logger->info("count: {}", queueFamilies[i].queueCount);
		logger->info("Flags:");
		printQueueFlags(queueFamilies[i]);
	}

	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

	// Check if all required physicalDevice extensions are available
	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(requiredDeviceExtension,
			[&availableDeviceExtensions](auto const& requiredDeviceExtension)
			{
				return std::ranges::any_of(availableDeviceExtensions,
					[requiredDeviceExtension](auto const& availableDeviceExtension)
					{ return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
			});

	// Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
	auto features =
		physicalDevice
		.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering && features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
		features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	// Return true if the physicalDevice meets all the criteria
	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

void Mupfel::Renderer::createLogicalDevice() {
	// find the index of the first queue family that supports graphics
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	// get the first index into queueFamilyProperties which supports both graphics and present
	uint32_t qIndex = ~0U;
	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
		{
			// found a queue family that supports both graphics and present
			qIndex = qfpIndex;
			break;
		}
	}
	if (qIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}
	queueIndex = qIndex;

	// query for Vulkan 1.3 features
	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
		{},                                   // vk::PhysicalDeviceFeatures2
		{.synchronization2 = true, .dynamicRendering = true},           // vk::PhysicalDeviceVulkan13Features
		{.extendedDynamicState = true}        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	};

	// create a Device
	float                     queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
	vk::DeviceCreateInfo      deviceCreateInfo{ .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
											   .queueCreateInfoCount = 1,
											   .pQueueCreateInfos = &deviceQueueCreateInfo,
											   .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
											   .ppEnabledExtensionNames = requiredDeviceExtension.data() };

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	queue = vk::raii::Queue(device, queueIndex, 0);
}

void Renderer::createSurface() {
#if 0
	VkSurfaceKHR       _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
#endif
}

vk::SurfaceFormatKHR Mupfel::Renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
	const auto formatIt = std::ranges::find_if(
		availableFormats,
		[](const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR Mupfel::Renderer::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
{
	assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(availablePresentModes,
		[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
		vk::PresentModeKHR::eMailbox :
		vk::PresentModeKHR::eFifo;
}

vk::Extent2D Mupfel::Renderer::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	int width = 0, height = 0;
	//glfwGetFramebufferSize(window, &width, &height);

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

void Mupfel::Renderer::createSwapChain()
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	std::vector<vk::PresentModeKHR> avaiablePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	swapChainExtent = chooseSwapExtent(surfaceCapabilities);
	swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);
	vk::PresentModeKHR chosenPresentMode = chooseSwapPresentMode(avaiablePresentModes);
	uint32_t imageCount = chooseSwapMinImageCount(surfaceCapabilities);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = swapChainSurfaceFormat.format,
		.imageColorSpace = swapChainSurfaceFormat.colorSpace,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = chosenPresentMode,
		.clipped = true };
	swapChainCreateInfo.oldSwapchain = nullptr;
	swapChainCreateInfo.imageExtent = swapChainExtent;

	swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
	swapChainImages = swapChain.getImages();
}

void Mupfel::Renderer::createImageViews()
{
	assert(swapChainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo = {
		.viewType = vk::ImageViewType::e2D,
		.format = swapChainSurfaceFormat.format,
		.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
	};

	for (auto& image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
}

bool Mupfel::Renderer::checkIfExtensionIsSupported(const char* const extension)
{
	auto extensionProperties = context.enumerateInstanceExtensionProperties();
	return std::ranges::any_of(extensionProperties,
		[extension](auto const& extensionProperty)
		{ return strcmp(extensionProperty.extensionName, extension) == 0; });
}

bool Mupfel::Renderer::checkIfRequiredLayersAreSupported(const std::vector<vk::LayerProperties>& layers)
{
	// Get the required layers
	std::vector<char const*> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	// Check if the required layers are supported by the Vulkan implementation.

	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
		[&layers](auto const& requiredLayer) {
			return std::ranges::none_of(layers,
				[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
		});
	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
	}
	return false;
}

uint32_t Mupfel::Renderer::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
{
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
	if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = surfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}

void Mupfel::Renderer::createGrahpicsPipeline()
{
	std::vector<char> shader_code = readFile("./Shaders/slang.spv");

	logger->info("Loaded {} bytes from file.", shader_code.size());

	vk::raii::ShaderModule shader_module = createShaderModule(shader_code);

	vk::PipelineShaderStageCreateInfo vertex_create;
	vertex_create.stage = vk::ShaderStageFlagBits::eVertex;
	vertex_create.module = shader_module;
	vertex_create.pName = "vertMain";

	vk::PipelineShaderStageCreateInfo fragment_create;
	fragment_create.stage = vk::ShaderStageFlagBits::eFragment;
	fragment_create.module = shader_module;
	fragment_create.pName = "fragMain";

	vk::PipelineShaderStageCreateInfo shader_stages[] = { vertex_create, fragment_create };

	vk::PipelineVertexInputStateCreateInfo vertex_input_info;

	/* Vertex input buffer settings */
	vk::PipelineInputAssemblyStateCreateInfo input_assembly;

	input_assembly.topology = vk::PrimitiveTopology::eTriangleList;

	/* Dynamic viewport and scissor states */
	vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height) , 0.0f, 1.0f };

	vk::Rect2D scissor{ vk::Offset2D{0,0}, swapChainExtent };

	std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data() };

	vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };

	/* Rasterizer settings */
	vk::PipelineRasterizationStateCreateInfo rasterizer{ .depthClampEnable = vk::False,
													.rasterizerDiscardEnable = vk::False,
													.polygonMode = vk::PolygonMode::eFill,
													.cullMode = vk::CullModeFlagBits::eBack,
													.frontFace = vk::FrontFace::eClockwise,
													.depthBiasEnable = vk::False,
													.lineWidth = 1.0f };

	/* Multisampling settings */
	vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };

	/* Color blending settings */
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
	.blendEnable = vk::True,
	.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
	.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
	.colorBlendOp = vk::BlendOp::eAdd,
	.srcAlphaBlendFactor = vk::BlendFactor::eOne,
	.dstAlphaBlendFactor = vk::BlendFactor::eZero,
	.alphaBlendOp = vk::BlendOp::eAdd,
	.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };

	vk::PipelineColorBlendStateCreateInfo colorBlending{
	.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment };

	/* Create empty pipeline layout */
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount = 0, .pushConstantRangeCount = 0 };
	pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
	{.stageCount = 2,
	 .pStages = shader_stages,
	 .pVertexInputState = &vertex_input_info,
	 .pInputAssemblyState = &input_assembly,
	 .pViewportState = &viewportState,
	 .pRasterizationState = &rasterizer,
	 .pMultisampleState = &multisampling,
	 .pColorBlendState = &colorBlending,
	 .pDynamicState = &dynamicState,
	 .layout = pipelineLayout,
	 .renderPass = nullptr},
	{.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format} };

	/* Create the graphics pipeline */
	graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

vk::raii::ShaderModule Mupfel::Renderer::createShaderModule(const std::vector<char>& code) const
{
	vk::ShaderModuleCreateInfo info;
	info.codeSize = code.size() * sizeof(char);
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());
	vk::raii::ShaderModule module(device, info);

	return module;
}

void Mupfel::Renderer::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = queueIndex;

	commandPool = vk::raii::CommandPool(device, poolInfo);
}

void Mupfel::Renderer::createCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo{ .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
	commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
}

void Mupfel::Renderer::recordCommandBuffer(uint32_t imageIndex)
{
	commandBuffer.begin({});

	// Transition the image layout for rendering
	transitionImageLayout(
		imageIndex,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	);

	// Set up the color attachment
	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::RenderingAttachmentInfo attachmentInfo = {
			.imageView = swapChainImageViews[imageIndex],
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = clearColor };

	// Set up the rendering info
	vk::RenderingInfo renderingInfo = {
			.renderArea = {.offset = {0, 0}, .extent = swapChainExtent},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo };

	// Begin rendering
	commandBuffer.beginRendering(renderingInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

	commandBuffer.draw(3, 1, 0, 0);

	// Rendering commands will go here

	// End rendering
	commandBuffer.endRendering();

	// Transition the image layout for presentation
	transitionImageLayout(
		imageIndex,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eBottomOfPipe
	);

	commandBuffer.end();
}

void Mupfel::Renderer::drawFrame()
{
	auto res = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
	if (res != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to wait for draw fence!");
	}
	device.resetFences(*drawFence);

	vk::ResultValue<uint32_t> image_index = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);

	recordCommandBuffer(image_index.value);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	const vk::SubmitInfo submitInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*presentCompleteSemaphore,
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &*commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*renderFinishedSemaphores[image_index.value] };
	queue.submit(submitInfo, *drawFence);

	const vk::PresentInfoKHR presentInfoKHR{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*renderFinishedSemaphores[image_index.value],
		.swapchainCount = 1,
		.pSwapchains = &*swapChain,
		.pImageIndices = &image_index.value };

	vk::Result present_res = queue.presentKHR(presentInfoKHR);
	(void)present_res;

}

void Mupfel::Renderer::createSyncObjects()
{
	presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
	/* Create a semaphore for each swapchain image */
	for (uint32_t i = 0; i < swapChainImages.size(); i++)
	{
		renderFinishedSemaphores.push_back(vk::raii::Semaphore(device, vk::SemaphoreCreateInfo()));
	}
	drawFence = vk::raii::Fence(device, { .flags = vk::FenceCreateFlagBits::eSignaled });
}

void Mupfel::Renderer::transitionImageLayout(
	uint32_t                imageIndex,
	vk::ImageLayout         old_layout,
	vk::ImageLayout         new_layout,
	vk::AccessFlags2        src_access_mask,
	vk::AccessFlags2        dst_access_mask,
	vk::PipelineStageFlags2 src_stage_mask,
	vk::PipelineStageFlags2 dst_stage_mask)
{
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = src_stage_mask,
		.srcAccessMask = src_access_mask,
		.dstStageMask = dst_stage_mask,
		.dstAccessMask = dst_access_mask,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = swapChainImages[imageIndex],
		.subresourceRange = {
			   .aspectMask = vk::ImageAspectFlagBits::eColor,
			   .baseMipLevel = 0,
			   .levelCount = 1,
			   .baseArrayLayer = 0,
			   .layerCount = 1} };
	vk::DependencyInfo dependencyInfo = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier };
	commandBuffer.pipelineBarrier2(dependencyInfo);
}