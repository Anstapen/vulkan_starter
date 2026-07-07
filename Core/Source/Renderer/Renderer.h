#pragma once

#include <cstdint>
#include <optional>

#include "Application/World.h"
#include "Logger/Logger.h"
#include "Ping/Buffer.h"
#include "Ping/Device.h"

namespace Mupfel
{

/**
 * Drives per-frame rendering on top of the `Ping` RHI: owns the swapchain, pipeline, command
 * buffers, and one vertex buffer per frame in flight, and syncs `Transform` components from the
 * `World` into those buffers each frame.
 */
class Renderer
{
public:
	/** Creates the swapchain, pipeline, command buffers, and per-frame-in-flight vertex buffers for `window`. */
	void Init(const Ping::Device& device, const Window& window);

	/**
	 * Renders and presents one frame: waits for the current frame-in-flight slot, syncs renderable
	 * entities to its vertex buffer, records and submits the draw, and presents. Recreates the
	 * swapchain (and returns early, skipping the rest of the frame) if it's found to be out of date.
	 */
	void RenderNextFrame(World& world, const Ping::Device& device, const Window& window);

	/** Currently a no-op; resource cleanup happens via RAII on destruction. */
	void Shutdown();

private:
	/** Advances `frameIndex` to the next frame-in-flight slot, wrapping at `frames_in_flight`. */
	void incrementFrameIndex();

	/** Copies every `Transform` in `world` into `vertex_buffers[frame_index]`'s mapped memory and flushes it. */
	void SyncRenderableObjects(World& world, const Ping::Device& device, uint32_t frame_index);

private:
	/** Number of frames pipelined in parallel. */
	static constexpr uint32_t frames_in_flight = 2;
	/** Current frame-in-flight slot, in `[0, frames_in_flight)`. */
	uint32_t frameIndex = 0;
	/** Logger created in `Init`. */
	Logger::SafeLoggerPtr logger;
	/** Empty until `Init` runs. */
	std::optional<Ping::SwapChain> swapchain;
	/** Empty until `Init` runs. */
	std::optional<Ping::Pipeline> pipeline;
	/** One per frame in flight; empty until `Init` runs. */
	std::optional<Ping::CommandBuffers> commandBuffers;
	/** One host-visible vertex buffer per frame in flight. */
	std::vector<Ping::Buffer> vertex_buffers;
};

} // namespace Mupfel
