#include "Vulkan/VulkanContext.h"
#include "Vulkan/VKManager.h"
#include "Vulkan/VKUtil.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanPipeline.h"
#include "Device.h"

#include <algorithm>
#include <limits>
#include <cstdint>

using namespace Ping;

Ping::Device::~Device()
{
}

Ping::Device::Device(const DeviceSpecification& specification, const Window& window)
{
	Backend::VKManager::Init();

	/* define the wanted queues */
	std::vector<Backend::VKQueueFamilyProperties> wanted_queues = {
		{vk::QueueFlagBits::eCompute, 1},
		{vk::QueueFlagBits::eGraphics, 1},
		{vk::QueueFlagBits::eTransfer, 1}
	};

	/* define the wanted extensions */
	std::vector<const char*> wanted_extensions = {
		vk::KHRSwapchainExtensionName
	};

	/* define the wanted validation layers */
	std::vector<const char*> wanted_validation_layers = {};

	vulkanContextPtr = std::make_unique<Backend::VulkanContext>(Backend::VKManager::CreateVulkanContext(window, wanted_queues, wanted_extensions, wanted_validation_layers));
}

Ping::Device::Device(Device&& other) : vulkanContextPtr(std::move(other.vulkanContextPtr))
{
}

Device& Ping::Device::operator=(Device&& other)
{
	this->vulkanContextPtr = std::move(other.vulkanContextPtr);
	return *this;
}

SwapChain Ping::Device::CreateSwapChain(const Window& window)
{
	auto vulkanSwapChain = Backend::VKManager::CreateSwapChain(*vulkanContextPtr.get(), window);

	return SwapChain(std::move(vulkanSwapChain));
}
Pipeline Ping::Device::CreatePipeline(const PipelineSpecification& specification, const SwapChain& swapchain)
{
	Backend::VulkanPipeline vulkanPipeline = Backend::VKManager::CreatePipeline(*vulkanContextPtr.get(), specification, *swapchain.vulkanSwapChainPtr.get());
	return Pipeline(std::move(vulkanPipeline));
}