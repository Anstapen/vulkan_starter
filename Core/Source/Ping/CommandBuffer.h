#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include "SwapChain.h"
#include "Types.h"
#include "Pipeline.h"

namespace Backend {
	class VulkanCommandBuffer;
}

namespace Ping
{

	class CommandBuffer
	{
	public:
		CommandBuffer(Backend::VulkanCommandBuffer && in_cmd_buffer) noexcept;
		CommandBuffer(const CommandBuffer& other) = delete;
		CommandBuffer(CommandBuffer&& other) noexcept;
		CommandBuffer& operator=(const CommandBuffer& other) = delete;
		CommandBuffer& operator=(CommandBuffer&& other) noexcept;
		~CommandBuffer() noexcept;
	public:
		void WaitForFences(const Device& device);
		void Begin(const Device &device);
		void transitionImageLayout(SwapChain& swapchain, uint32_t image_index, const ImageLayoutTransition &transition);
		void BeginRendering(SwapChain & swapchain, uint32_t image_index);
		void BindPipeline(Pipeline& pipeline);
		void Submit(const Device& device, const SwapChain& swapchain, uint32_t frameIndex, uint32_t imageIndex);
		void Draw() const;
		void EndRendering() const;
		void End() const;
	private:
		std::unique_ptr<Backend::VulkanCommandBuffer> vulkanCommandBufferPtr;
	};

	typedef std::vector<CommandBuffer> CommandBuffers;

}