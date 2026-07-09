#include "VulkanCommandBuffer.h"
#include "VKManager.h"

using namespace Backend;

Backend::VulkanCommandBuffer::VulkanCommandBuffer(
	vk::raii::CommandBuffer&& in_cmd_buffer,
	vk::raii::Fence&&		  in_draw_fence) noexcept
	: commandBuffer(std::move(in_cmd_buffer)), drawFence(std::move(in_draw_fence))
{
}

Backend::VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept
	: commandBuffer(std::move(other.commandBuffer)), drawFence(std::move(other.drawFence))
{
}

VulkanCommandBuffer& Backend::VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) noexcept
{
	commandBuffer = std::move(other.commandBuffer);
	drawFence = std::move(other.drawFence);
	return *this;
}

void VulkanCommandBuffer::Begin(const vk::raii::Device& device, vk::CommandBufferUsageFlags usage) const
{
	device.resetFences(*drawFence);

	vk::CommandBufferBeginInfo info{};
	info.flags = usage;

	commandBuffer.begin(info);
}

void Backend::VulkanCommandBuffer::WaitForFences(const vk::raii::Device& device) const
{
	auto fenceResult = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}
}

void Backend::VulkanCommandBuffer::BindPipeline(const VulkanPipeline& pipeline) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.pipeline);
}

void Backend::VulkanCommandBuffer::BindVertexBuffer(
	const VulkanPipeline& pipeline,
	const VulkanBuffer&	  buffer,
	uint32_t			  binding) const
{

	commandBuffer.bindVertexBuffers(binding, *buffer.buffer, {0});
}

void Backend::VulkanCommandBuffer::BindIndexBuffer(const VulkanPipeline& pipeline, const VulkanBuffer& buffer) const
{
	commandBuffer.bindIndexBuffer(*buffer.buffer, 0, vk::IndexType::eUint16);
}

void Backend::VulkanCommandBuffer::BindDescriptorSet(
	const VulkanPipeline&		pipeline,
	const VulkanDescriptorPool& descriptor_pool,
	uint32_t					frame_index) const
{
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, *pipeline.pipelineLayout, 0, *descriptor_pool.sets[frame_index], {});
}

void VulkanCommandBuffer::Draw(uint32_t vertex_count) const { commandBuffer.draw(vertex_count, 1, 0, 0); }

void Backend::VulkanCommandBuffer::DrawIndexed(uint32_t index_count) const
{
	commandBuffer.drawIndexed(index_count, 1, 0, 0, 0);
}

void Backend::VulkanCommandBuffer::Submit(
	VulkanContext&	 context,
	VulkanSwapChain& swapchain,
	uint32_t		 frameIndex,
	uint32_t		 imageIndex) const
{
	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	vk::SubmitInfo		   submitInfo{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*swapchain.presentCompleteSemaphores[frameIndex],
				.pWaitDstStageMask = &waitDestinationStageMask,
				.commandBufferCount = 1,
				.pCommandBuffers = &*commandBuffer,
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = &*swapchain.renderFinishedSemaphores[imageIndex]};

	/* retrieve the correct queue */
	uint32_t q_index = VKManager::GetQueueIndex(context, Ping::QueueType::Graphics);

	context.queues[q_index].queue.submit(submitInfo, *drawFence);
}

void Backend::VulkanCommandBuffer::EndRendering() const { commandBuffer.endRendering(); }

void Backend::VulkanCommandBuffer::End() const { commandBuffer.end(); }
