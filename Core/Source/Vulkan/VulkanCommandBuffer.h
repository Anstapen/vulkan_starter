#pragma once
#include "VulkanCommon.h"
#include "VulkanPipeline.h"
#include "VulkanSwapChain.h"
#include <vector>

namespace Backend {
	class VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(vk::raii::CommandBuffer&& in_cmd_buffer, vk::raii::Fence &&in_draw_fence) noexcept;
		VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept;
		VulkanCommandBuffer& operator=(const VulkanCommandBuffer& other) = delete;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&& other) noexcept;
		~VulkanCommandBuffer() = default;
	public:
		void WaitForFences(const vk::raii::Device& device) const;
		void Begin(const vk::raii::Device& device) const;
		void BindPipeline(const VulkanPipeline& pipeline) const;
		void Draw() const;
		void Submit(VulkanContext &context, VulkanSwapChain& swapchain, uint32_t frameIndex, uint32_t imageIndex) const;
		void EndRendering() const;
		void End() const;
	public:
		vk::raii::CommandBuffer commandBuffer;
		vk::raii::Fence			drawFence;
	};

	typedef std::vector<VulkanCommandBuffer> VulkanCommandBuffers;
}