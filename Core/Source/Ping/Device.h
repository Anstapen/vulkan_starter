#pragma once
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Pipeline.h"
#include "SwapChain.h"
#include "Types.h"
#include "Window/Window.h"
#include <memory>

/* Forward Declaration of backend-specific classes and types */
namespace Backend
{
class VulkanContext;
}

namespace Ping
{

/** Reserved for future device selection/creation options; currently empty. */
struct DeviceSpecification
{
};

/**
 * The entry point of the Ping RHI: owns the Vulkan instance/physical/logical device and queues, and
 * is the factory for every other `Ping` resource (`SwapChain`, `Pipeline`, `CommandBuffers`, `Buffer`).
 *
 * @note Move-only: all resources created from a `Device` (swapchains, pipelines, command buffers)
 * reference it and must not outlive it.
 */
class Device
{
	friend class CommandBuffer;
	friend class SwapChain;

public:
	virtual ~Device();

	/** Initializes the Vulkan backend (instance, physical/logical device, queues) for `window`. */
	Device(const DeviceSpecification& specification, const Window& window);
	Device(const Device& other) = delete;
	/** Move-constructs from `other`, taking over its Vulkan context. */
	Device(Device&& other);
	Device& operator=(const Device& other) = delete;
	/** Move-assigns from `other`, taking over its Vulkan context. */
	Device& operator=(Device&& other);

public:
	/** Creates a swapchain that presents to `window`, pipelined across `frames_in_flight` frames. */
	SwapChain CreateSwapChain(const Window& window, uint32_t frames_in_flight) const;

	/** Builds a graphics pipeline per `specification`, targeting `swapchain`'s image format. */
	Pipeline CreatePipeline(const PipelineSpecification& specification, const SwapChain& swapchain) const;

	/**
	 * Allocates `num_buffers` command buffers from the given queue family.
	 *
	 * @return An empty `CommandBuffers` if `num_buffers` is 0.
	 * @throws std::runtime_error if the backend fails to allocate any command buffers.
	 */
	CommandBuffers CreateCommandBuffers(QueueType buffer_type, uint32_t num_buffers) const;

	/** Allocates a GPU buffer of `size` bytes with the given usage and memory properties. */
	Buffer CreateBuffer(size_t size, BufferUsage usage, MemoryProperty property) const;

	/** Blocks until the device has completed all previously submitted work. */
	void WaitForCommands() const;

	/**
	 * Flushes host writes to `buffer` so they are visible to the device (needed unless the buffer
	 * was created with `MemoryProperty::HostCoherent`).
	 */
	void Flush(const Buffer& buffer) const;

private:
	/** Owning pointer to the Vulkan instance/device/queues. */
	std::unique_ptr<Backend::VulkanContext> vulkanContextPtr;
};

} // namespace Ping