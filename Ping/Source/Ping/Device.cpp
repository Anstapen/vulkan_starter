#include "Device.h"
#include "Vulkan/VKManager.h"
#include "Vulkan/VKUtil.h"
#include "Vulkan/VulkanBuffer.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanGui.h"
#include "Vulkan/VulkanImage.h"
#include "Vulkan/VulkanPipeline.h"
#include "Vulkan/VulkanSampler.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanTypeConversions.h"

#include <algorithm>
#include <cstdint>
#include <limits>

using namespace Ping;

Ping::Device::~Device() {}

Ping::Device::Device(const DeviceSpecification& specification, GLFWwindow* window)
{
	(void)specification;
	Backend::VKManager::Init();

	/* define the wanted queues */
	std::vector<Backend::VKQueueRequest> wanted_queues = {
		{Ping::QueueType::Graphics, /*prefer_dedicated_family=*/false},
		{Ping::QueueType::Compute, /*prefer_dedicated_family=*/true},
		{Ping::QueueType::Transfer, /*prefer_dedicated_family=*/true},
	};

	/* define the wanted extensions */
	std::vector<const char*> wanted_device_extensions = {
		vk::KHRSwapchainExtensionName, vk::KHRSynchronization2ExtensionName, vk::EXTExtendedDynamicStateExtensionName};

/* define the wanted validation layers */
#ifndef NDEBUG
	std::vector<const char*> wanted_validation_layers = {"VK_LAYER_KHRONOS_validation"};
#else
	std::vector<const char*> wanted_validation_layers = {};
#endif

	vulkanContextPtr = std::make_unique<Backend::VulkanContext>(Backend::VKManager::CreateVulkanContext(
		window, wanted_queues, {}, wanted_device_extensions, wanted_validation_layers));
}

Ping::Device::Device(Device&& other) : vulkanContextPtr(std::move(other.vulkanContextPtr)) {}

Device& Ping::Device::operator=(Device&& other)
{
	this->vulkanContextPtr = std::move(other.vulkanContextPtr);
	return *this;
}

SwapChain Ping::Device::CreateSwapChain(GLFWwindow* window, uint32_t frames_in_flight) const
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
	Backend::VulkanBuffer buffer = Backend::VKManager::CreateBuffer(
		*vulkanContextPtr.get(), size, Backend::ToVulkan(usage), Backend::ToVulkan(property));
	return Buffer(std::move(buffer));
}

std::optional<Image> Ping::Device::CreateImage(const std::string& path, Ping::ImageUsage usage) const
{
	std::optional<Backend::VulkanImage> vk_image =
		Backend::VKManager::LoadVulkanImage(*vulkanContextPtr.get(), path, Backend::ToVulkan(usage));

	if (!vk_image.has_value())
	{
		return {};
	}

	return {std::move(vk_image)};
}

Sampler Ping::Device::CreateSampler(const SamplerSpecification& sampler_spec) const
{
	Backend::VulkanSampler sampler = Backend::VKManager::CreateSampler(*vulkanContextPtr.get(), sampler_spec);
	return Sampler(std::move(sampler));
}

DescriptorSets Ping::Device::CreateDescriptorSets(
	const Pipeline&			   pipeline,
	uint32_t				   set_index,
	const std::vector<Buffer>& uniform_buffers) const
{
	std::vector<const Backend::VulkanBuffer*> backend_buffers;
	backend_buffers.reserve(uniform_buffers.size());

	for (const auto& buffer : uniform_buffers)
	{
		backend_buffers.push_back(buffer.vulkanBufferPtr.get());
	}

	Backend::VulkanDescriptorPool pool = Backend::VKManager::CreateUBODescriptorSets(
		*vulkanContextPtr.get(), *pipeline.vulkanPipelinePtr.get(), set_index, backend_buffers);

	return DescriptorSets(std::move(pool));
}

DescriptorSets Ping::Device::CreateSamplerDescriptorSets(
	const Pipeline&											  pipeline,
	uint32_t												  set_index,
	const std::vector<Image>&								  images,
	const std::vector<std::reference_wrapper<const Sampler>>& samplers) const
{
	std::vector<const Backend::VulkanImage*> backend_images;
	backend_images.reserve(images.size());
	for (const auto& image : images)
	{
		backend_images.push_back(image.vulkanImagePtr.get());
	}

	std::vector<const Backend::VulkanSampler*> backend_samplers;
	backend_samplers.reserve(samplers.size());
	for (const auto& sampler : samplers)
	{
		backend_samplers.push_back(sampler.get().vulkanSamplerPtr.get());
	}

	Backend::VulkanDescriptorPool pool = Backend::VKManager::CreateSamplerDescriptorSets(
		*vulkanContextPtr.get(), *pipeline.vulkanPipelinePtr.get(), set_index, backend_images, backend_samplers);

	return DescriptorSets(std::move(pool));
}

DescriptorSets Ping::Device::CreateStorageDescriptorSets(
	const Pipeline&			   pipeline,
	uint32_t				   set_index,
	const std::vector<Buffer>& storage_buffers) const
{
	std::vector<const Backend::VulkanBuffer*> backend_buffers;
	backend_buffers.reserve(storage_buffers.size());

	for (const auto& buffer : storage_buffers)
	{
		backend_buffers.push_back(buffer.vulkanBufferPtr.get());
	}

	Backend::VulkanDescriptorPool pool = Backend::VKManager::CreateStorageDescriptorSets(
		*vulkanContextPtr.get(), *pipeline.vulkanPipelinePtr.get(), set_index, backend_buffers);

	return DescriptorSets(std::move(pool));
}

Gui Ping::Device::CreateGui(GLFWwindow* window, const SwapChain& swapchain, uint32_t frames_in_flight) const
{
	Backend::VulkanGui vulkanGui = Backend::VKManager::CreateGui(
		*vulkanContextPtr.get(), window, *swapchain.vulkanSwapChainPtr.get(), frames_in_flight);
	return Gui(std::move(vulkanGui));
}

void Ping::Device::WaitForCommands() const { Backend::VKManager::WaitForCommands(vulkanContextPtr->device); }