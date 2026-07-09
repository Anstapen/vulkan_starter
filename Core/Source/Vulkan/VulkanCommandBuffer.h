#pragma once
#include "VulkanBuffer.h"
#include "VulkanCommon.h"
#include "VulkanDescriptorPool.h"
#include "VulkanPipeline.h"
#include "VulkanSwapChain.h"
#include <vector>

namespace Backend
{

/**
 * RAII-owning backend counterpart of `Ping::CommandBuffer`: the command buffer itself plus the
 * fence used to know when the GPU has finished with it, so its slot can be reused.
 *
 * @note Move-only. Built by `VKManager::CreateCommandBuffers`; not intended to be constructed
 * directly elsewhere.
 */
class VulkanCommandBuffer
{
public:
	/**
	 * `in_draw_fence` should be created already signaled, so the first `WaitForFences` for this
	 * slot doesn't block waiting on a submission that never happened.
	 */
	VulkanCommandBuffer(vk::raii::CommandBuffer&& in_cmd_buffer, vk::raii::Fence&& in_draw_fence) noexcept;
	VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
	/** Move-constructs from `other`, taking over its command buffer and fence. */
	VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept;
	VulkanCommandBuffer& operator=(const VulkanCommandBuffer& other) = delete;
	/** Move-assigns from `other`, taking over its command buffer and fence. */
	VulkanCommandBuffer& operator=(VulkanCommandBuffer&& other) noexcept;
	~VulkanCommandBuffer() = default;

public:
	/**
	 * Blocks until `drawFence` is signaled, i.e. until the GPU finishes the previous submission using this slot.
	 * @throws std::runtime_error if the wait doesn't complete with `vk::Result::eSuccess`.
	 */
	void WaitForFences(const vk::raii::Device& device) const;

	/**
	 * Resets `drawFence` and begins recording. Callers must have called `WaitForFences` first, so the
	 * fence isn't reset while a submission that references it could still be pending.
	 */
	void Begin(const vk::raii::Device& device, vk::CommandBufferUsageFlags usage) const;

	/** Binds `pipeline` at the graphics bind point. */
	void BindPipeline(const VulkanPipeline& pipeline) const;

	/** Binds `buffer` at vertex input `binding`, starting at offset 0. */
	void BindVertexBuffer(const VulkanPipeline& pipeline, const VulkanBuffer& buffer, uint32_t binding) const;

	/** Binds `buffer` as index buffer. */
	void BindIndexBuffer(const VulkanPipeline& pipeline, const VulkanBuffer& buffer) const;

	/** Binds `descriptor_pool`'s set for `frame_index` at the graphics bind point, using `pipeline`'s layout. */
	void BindDescriptorSet(
		const VulkanPipeline&		pipeline,
		const VulkanDescriptorPool& descriptor_pool,
		uint32_t					frame_index) const;

	/** Records a non-indexed draw of `vertex_count` vertices, 1 instance, starting at vertex/instance 0. */
	void Draw(uint32_t vertex_count) const;

	/** Records an indexed draw of `index_count` vertices, 1 instance, starting at vertex/instance 0. */
	void DrawIndexed(uint32_t index_count) const;

	/**
	 * Submits to the context's graphics queue: waits on `swapchain.presentCompleteSemaphores[frameIndex]`
	 * at the color-attachment-output stage, signals `swapchain.renderFinishedSemaphores[imageIndex]`, and
	 * signals `drawFence` on completion.
	 */
	void Submit(VulkanContext& context, VulkanSwapChain& swapchain, uint32_t frameIndex, uint32_t imageIndex) const;

	/** Ends the dynamic rendering pass. */
	void EndRendering() const;

	/** Ends recording. */
	void End() const;

public:
	/** The underlying Vulkan command buffer. */
	vk::raii::CommandBuffer commandBuffer;
	/** Signaled when the GPU finishes the submission that used this buffer. */
	vk::raii::Fence drawFence;
};

/** One command buffer per frame in flight, as returned by `VKManager::CreateCommandBuffers`. */
typedef std::vector<VulkanCommandBuffer> VulkanCommandBuffers;
} // namespace Backend