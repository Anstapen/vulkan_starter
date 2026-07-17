#pragma once
#include "Buffer.h"
#include "DescriptorSets.h"
#include "Gui.h"
#include "Pipeline.h"
#include "SwapChain.h"
#include "Image.h"
#include "Types.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace Backend
{
class VulkanCommandBuffer;
}

namespace Ping
{

/**
 * An RAII-owned, per-frame-in-flight command buffer, created via `Device::CreateCommandBuffers`.
 *
 * Expected per-frame call order: `WaitForFences` (to reuse this slot safely), `Begin`,
 * `transitionImageLayout` (to `ColorAttachmentOptimal`), `BeginRendering`, `BindPipeline`,
 * `BindVertexBuffer`, `Draw`, `EndRendering`, `transitionImageLayout` (to `PresentSource`), `End`,
 * `Submit`. See `Mupfel::Renderer::RenderNextFrame` for a worked example.
 *
 * @note Move-only: owns the backend command buffer and its associated fence for its lifetime.
 */
class CommandBuffer
{
public:
	/** Takes ownership of an existing backend command buffer. Used internally by `Device::CreateCommandBuffers`. */
	CommandBuffer(Backend::VulkanCommandBuffer&& in_cmd_buffer) noexcept;
	CommandBuffer(const CommandBuffer& other) = delete;
	/** Move-constructs from `other`, taking over its backend command buffer. */
	CommandBuffer(CommandBuffer&& other) noexcept;
	CommandBuffer& operator=(const CommandBuffer& other) = delete;
	/** Move-assigns from `other`, taking over its backend command buffer. */
	CommandBuffer& operator=(CommandBuffer&& other) noexcept;
	~CommandBuffer() noexcept;

public:
	/** Blocks until the GPU has finished the previous submission that used this command buffer's fence. */
	void WaitForFences(const Device& device);

	/** Begins recording. Must be called before any other recording method, once per frame. */
	void Begin(const Device& device, CommandBufferUsage usage);

	/** Records a pipeline barrier that transitions `swapchain`'s image at `image_index` per `transition`. */
	void transitionImageLayout(SwapChain& swapchain, uint32_t image_index, const ImageLayoutTransition& transition);

	void transitionImageLayout(Image& image, const ImageLayoutTransition& transition);

	/**
	 * Begins dynamic rendering into `swapchain`'s image at `image_index`. Requires a prior transition to
	 * `ImageLayout::ColorAttachmentOptimal`.
	 */
	void BeginRendering(SwapChain& swapchain, Image& depth_buffer, uint32_t image_index);

	/** Binds `pipeline` for subsequent draw calls. */
	void BindPipeline(Pipeline& pipeline);

	/** Binds `buffer` as the vertex buffer for `pipeline`'s given `binding` slot. */
	void BindVertexBuffer(const Buffer& buffer, uint32_t binding) const;

	/** Binds `buffer` as the index buffer for `pipeline`'. */
	void BindIndexBuffer(const Buffer& buffer) const;

	/** Binds `descriptor_sets`' set at `set_element` to descriptor set slot `set_index` for `pipeline`. */
	void BindDescriptorSet(
		const Pipeline&		  pipeline,
		const DescriptorSets& descriptor_sets,
		uint32_t			  set_element,
		uint32_t			  set_index) const;

	void DrawGui(const Device& device, const Gui& gui, uint32_t frame_index) const;

	/**
	 * Submits the recorded commands to the device's graphics queue. `frameIndex` is this command
	 * buffer's slot among the frames in flight; `imageIndex` is the swapchain image acquired via
	 * `SwapChain::AcquireNextImage`.
	 */
	void Submit(const Device& device, const SwapChain& swapchain, uint32_t frameIndex, uint32_t imageIndex);

	/** Records a non-indexed draw of `vertex_count` vertices from the currently bound vertex buffer. */
	void Draw(uint32_t vertex_count) const;

	/** Records an indexed draw of index_count vertices from the currently bound vertex buffer. */
	void DrawIndexed(uint32_t index_count, uint32_t instance_count) const;

	/** Ends the dynamic rendering pass started by `BeginRendering`. */
	void EndRendering() const;

	/** Ends recording. Must be called before `Submit`. */
	void End() const;

private:
	std::unique_ptr<Backend::VulkanCommandBuffer>
		/** Owning pointer to the backend command buffer. */
		vulkanCommandBufferPtr;
};

/** helper typedef */
typedef std::vector<CommandBuffer> CommandBuffers;

} // namespace Ping