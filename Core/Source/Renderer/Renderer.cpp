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

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

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
	checkInstanceExtensions();
	createInstance();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createGrahpicsPipeline();
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

	/* Check if required GLFW extensions are supported by the vulkan implementation */
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		if (!checkIfExtensionIsSupported(glfwExtensions[i]))
		{
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
		}
	}

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	std::vector<vk::LayerProperties> layerProperties = context.enumerateInstanceLayerProperties();
	checkIfRequiredLayersAreSupported(layerProperties);
	
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}
	
	instance = vk::raii::Instance(context, createInfo);
}

void Mupfel::Renderer::checkInstanceExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	if (glfwExtensionCount == 0 || glfwExtensions == NULL)
	{
		std::cout << "Error: could not retrieve the required GLFW extensions!" << std::endl;
		return;
	}

	std::cout << "The following Vulkan extensions are needed by GLFW:" << std::endl;
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		std::cout << glfwExtensions[i] << std::endl;
	}
}

std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName, "VK_KHR_shader_draw_parameters" };

static void printQueueFlags(const vk::QueueFamilyProperties& prop)
{
	vk::QueueFlags flags = prop.queueFlags;

	if (flags & vk::QueueFlagBits::eGraphics)
	{
		std::cout << "eGraphics" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eCompute)
	{
		std::cout << "eCompute" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eDataGraphARM)
	{
		std::cout << "eDataGraphARM" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eOpticalFlowNV)
	{
		std::cout << "eOpticalFlowNV" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eProtected)
	{
		std::cout << "eProtected" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eSparseBinding)
	{
		std::cout << "eSparseBinding" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eTransfer)
	{
		std::cout << "eTransfer" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eVideoDecodeKHR)
	{
		std::cout << "eVideoDecodeKHR" << std::endl;
	}

	if (flags & vk::QueueFlagBits::eVideoEncodeKHR)
	{
		std::cout << "eVideoEncodeKHR" << std::endl;
	}
}

bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice)
{
	vk::PhysicalDeviceProperties prop = physicalDevice.getProperties();
	std::cout << prop.deviceName.data() << ":" << std::endl;
	std::cout << "API version: " << vk::versionMajor(prop.apiVersion) << "." << vk::versionMinor(prop.apiVersion) << std::endl;
	std::cout << "Device Type: " << static_cast<uint32_t>(prop.deviceType) << std::endl;

	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = prop.apiVersion >= vk::ApiVersion13;

	// Check if any of the queue families support graphics operations
	auto queueFamilies = physicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		std::cout << "Queue " << i << ":" << std::endl;
		std::cout << "count: " << queueFamilies[i].queueCount << std::endl;
		std::cout << "Flags:" << std::endl;
		printQueueFlags(queueFamilies[i]);
		std::cout << std::endl;
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
	bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
		features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	// Return true if the physicalDevice meets all the criteria
	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

void Renderer::pickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

	auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) { return isDeviceSuitable(physicalDevice); });
	if (devIter == physicalDevices.end())
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	physicalDevice = *devIter;
}

void Mupfel::Renderer::createLogicalDevice() {
	// find the index of the first queue family that supports graphics
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	// get the first index into queueFamilyProperties which supports both graphics and present
	uint32_t queueIndex = ~0;
	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
		{
			// found a queue family that supports both graphics and present
			queueIndex = qfpIndex;
			break;
		}
	}
	if (queueIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// query for Vulkan 1.3 features
	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
		{},                                   // vk::PhysicalDeviceFeatures2
		{.dynamicRendering = true},           // vk::PhysicalDeviceVulkan13Features
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
	VkSurfaceKHR       _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
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
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

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

	std::cout << "Loaded " << shader_code.size() << " bytes from file." << std::endl;

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
	vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height) , 0.0f, 1.0f};

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
