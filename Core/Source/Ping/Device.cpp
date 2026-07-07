#include "Device.h"
#include "Vulkan/VKManager.h"
#include "Vulkan/VKUtil.h"
#include "Vulkan/VulkanBuffer.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanPipeline.h"
#include "Vulkan/VulkanSwapChain.h"

#include <algorithm>
#include <cstdint>
#include <limits>

using namespace Ping;

Ping::Device::~Device() {}

Ping::Device::Device(const DeviceSpecification& specification, const Window& window)
{
	(void)specification;
	Backend::VKManager::Init();

	/* define the wanted queues */
	std::vector<Backend::VKQueueFamilyProperties> wanted_queues = {
		{vk::QueueFlagBits::eCompute, 1}, {vk::QueueFlagBits::eGraphics, 1}, {vk::QueueFlagBits::eTransfer, 1}};

	/* define the wanted extensions */
	std::vector<const char*> wanted_extensions = {vk::KHRSwapchainExtensionName};

	/* define the wanted validation layers */
	std::vector<const char*> wanted_validation_layers = {};

	vulkanContextPtr = std::make_unique<Backend::VulkanContext>(
		Backend::VKManager::CreateVulkanContext(window, wanted_queues, wanted_extensions, wanted_validation_layers));
}

Ping::Device::Device(Device&& other) : vulkanContextPtr(std::move(other.vulkanContextPtr)) {}

Device& Ping::Device::operator=(Device&& other)
{
	this->vulkanContextPtr = std::move(other.vulkanContextPtr);
	return *this;
}

SwapChain Ping::Device::CreateSwapChain(const Window& window, uint32_t frames_in_flight) const
{
	auto vulkanSwapChain = Backend::VKManager::CreateSwapChain(*vulkanContextPtr.get(), window, frames_in_flight);

	return SwapChain(std::move(vulkanSwapChain));
}

Pipeline Ping::Device::CreatePipeline(const PipelineSpecification& specification, const SwapChain& swapchain) const
{
	Backend::VulkanPipeline vulkanPipeline =
		Backend::VKManager::CreatePipeline(*vulkanContextPtr.get(), specification, *swapchain.vulkanSwapChainPtr.get());
	return Pipeline(std::move(vulkanPipeline));
}

CommandBuffers Ping::Device::CreateCommandBuffers(QueueType buffer_type, uint32_t num_buffers) const
{
	if (num_buffers == 0)
	{
		return CommandBuffers();
	}

	auto backend_buffers = Backend::VKManager::CreateCommandBuffers(*vulkanContextPtr.get(), buffer_type, num_buffers);

	if (backend_buffers.size() == 0)
	{
		throw std::runtime_error("Unable to create buffers!");
	}

	CommandBuffers cmd_buffers;

	for (auto& buffer : backend_buffers)
	{
		cmd_buffers.emplace_back(std::move(buffer));
	}

	return cmd_buffers;
}

Buffer Ping::Device::CreateBuffer(size_t size, BufferUsage usage, MemoryProperty property) const
{
	Backend::VulkanBuffer buffer = Backend::VKManager::CreateBuffer(*vulkanContextPtr.get(), size, usage, property);
	return Buffer(std::move(buffer));
}

void Ping::Device::WaitForCommands() const { Backend::VKManager::WaitForCommands(vulkanContextPtr->device); }

void Ping::Device::Flush(const Buffer& buffer) const
{
	Backend::VKManager::FlushMappedMemoryRanges(*vulkanContextPtr.get(), *buffer.vulkanBufferPtr.get());
}
