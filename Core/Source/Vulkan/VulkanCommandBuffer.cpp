#include "VulkanCommandBuffer.h"
#include "VKManager.h"

using namespace Backend;

Backend::VulkanCommandBuffer::VulkanCommandBuffer(
	vk::raii::CommandBuffer&& in_cmd_buffer,
	vk::raii::Fence&& in_draw_fence) noexcept :
	commandBuffer(std::move(in_cmd_buffer)),
	drawFence(std::move(in_draw_fence))
{
}

Backend::VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept :
	commandBuffer(std::move(other.commandBuffer)),
	drawFence(std::move(other.drawFence))
{
}

VulkanCommandBuffer& Backend::VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) noexcept
{
	commandBuffer = std::move(other.commandBuffer);
	drawFence = std::move(other.drawFence);
	return *this;
}

void VulkanCommandBuffer::Begin(const vk::raii::Device& device) const
{
	auto fenceResult = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}
	device.resetFences(*drawFence);
	commandBuffer.begin({});
}

void Backend::VulkanCommandBuffer::BindPipeline(const VulkanPipeline& pipeline) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.pipeline);
}

void VulkanCommandBuffer::Draw() const
{
	commandBuffer.draw(3, 1, 0, 0);
}

void Backend::VulkanCommandBuffer::Submit(VulkanContext& context, VulkanSwapChain& swapchain)
{
	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	vk::SubmitInfo   submitInfo{.waitSemaphoreCount = 1,
								.pWaitSemaphores = &*swapchain.presentCompleteSemaphore,
								.pWaitDstStageMask = &waitDestinationStageMask,
								.commandBufferCount = 1,
								.pCommandBuffers = &*commandBuffer,
								.signalSemaphoreCount = 1,
								.pSignalSemaphores = &*swapchain.renderFinishedSemaphore };
	
	/* retrieve the correct queue */
	uint32_t q_index = VKManager::GetQueueIndex(context, Ping::QueueType::Graphics);

	context.queues[q_index].queue.submit(submitInfo, *drawFence);
}

void Backend::VulkanCommandBuffer::EndRendering() const
{
	commandBuffer.endRendering();
}

void Backend::VulkanCommandBuffer::End() const
{
	commandBuffer.end();
}
