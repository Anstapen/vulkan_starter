#include "CommandBuffer.h"
#include "Ping/Device.h"
#include "Vulkan/VKManager.h"
#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanTypeConversions.h"

using namespace Ping;

Ping::CommandBuffer::CommandBuffer(Backend::VulkanCommandBuffer&& in_cmd_buffer) noexcept
	: vulkanCommandBufferPtr(std::make_unique<Backend::VulkanCommandBuffer>(std::move(in_cmd_buffer)))
{
}

Ping::CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
	: vulkanCommandBufferPtr(std::move(other.vulkanCommandBufferPtr))
{
}

CommandBuffer& Ping::CommandBuffer::operator=(CommandBuffer&& other) noexcept
{
	vulkanCommandBufferPtr = std::move(other.vulkanCommandBufferPtr);
	return *this;
}

Ping::CommandBuffer::~CommandBuffer() {}

void Ping::CommandBuffer::WaitForFences(const Device& device)
{
	vulkanCommandBufferPtr->WaitForFences(device.vulkanContextPtr->device);
}

void Ping::CommandBuffer::Begin(const Device& device, CommandBufferUsage usage)
{

	vulkanCommandBufferPtr->Begin(device.vulkanContextPtr->device, Backend::ToVulkan(usage));
}

void Ping::CommandBuffer::transitionImageLayout(
	SwapChain&					 swapchain,
	uint32_t					 image_index,
	const ImageLayoutTransition& transition)
{
	Backend::VKManager::transitionImageLayout(
		*vulkanCommandBufferPtr.get(), *swapchain.vulkanSwapChainPtr.get(), image_index, transition);
}

void Ping::CommandBuffer::BeginRendering(SwapChain& swapchain, uint32_t image_index)
{
	Backend::VKManager::beginRendering(*vulkanCommandBufferPtr.get(), *swapchain.vulkanSwapChainPtr.get(), image_index);
}

void Ping::CommandBuffer::BindPipeline(Pipeline& pipeline)
{
	vulkanCommandBufferPtr->BindPipeline(*pipeline.vulkanPipelinePtr.get());
}

void Ping::CommandBuffer::BindVertexBuffer(const Pipeline& pipeline, const Buffer& buffer, uint32_t binding) const
{
	vulkanCommandBufferPtr->BindVertexBuffer(*pipeline.vulkanPipelinePtr.get(), *buffer.vulkanBufferPtr.get(), binding);
}

void Ping::CommandBuffer::Submit(
	const Device&	 device,
	const SwapChain& swapchain,
	uint32_t		 frameIndex,
	uint32_t		 imageIndex)
{
	vulkanCommandBufferPtr->Submit(
		*device.vulkanContextPtr.get(), *swapchain.vulkanSwapChainPtr.get(), frameIndex, imageIndex);
}

void Ping::CommandBuffer::Draw(uint32_t vertex_count) const { vulkanCommandBufferPtr->Draw(vertex_count); }

void Ping::CommandBuffer::End() const { vulkanCommandBufferPtr->End(); }

void Ping::CommandBuffer::EndRendering() const { vulkanCommandBufferPtr->EndRendering(); }