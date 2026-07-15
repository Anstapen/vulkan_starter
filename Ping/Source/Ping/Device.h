#pragma once
#include "Buffer.h"
#include "CommandBuffer.h"
#include "DescriptorSets.h"
#include "Gui.h"
#include "Image.h"
#include "Pipeline.h"
#include "Sampler.h"
#include "SwapChain.h"
#include "Types.h"
#include <functional>
#include <memory>
#include <optional>

/* forward declaration to avoid pulling GLFW headers into this public-facing header */
struct GLFWwindow;

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
	friend class Buffer;
	friend class Image;
	friend class Sampler;

public:
	virtual ~Device();

	/** Initializes the Vulkan backend (instance, physical/logical device, queues) for `window`. */
	Device(const DeviceSpecification& specification, GLFWwindow* window);
	Device(const Device& other) = delete;
	/** Move-constructs from `other`, taking over its Vulkan context. */
	Device(Device&& other);
	Device& operator=(const Device& other) = delete;
	/** Move-assigns from `other`, taking over its Vulkan context. */
	Device& operator=(Device&& other);

public:
	/** Creates a swapchain that presents to `window`, pipelined across `frames_in_flight` frames. */
	SwapChain CreateSwapChain(GLFWwindow* window, uint32_t frames_in_flight) const;

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

	/** Allocates a GPU buffer of `size` bytes with the given usage and memory properties. */
	std::optional<Image> CreateImage(const std::string& path, Ping::ImageUsage usage) const;

	Sampler CreateSampler(const SamplerSpecification& sampler_spec) const;

	DescriptorSets CreateDescriptorSets(
		const Pipeline&			   pipeline,
		uint32_t				   set_index,
		const std::vector<Buffer>& uniform_buffers) const;

	DescriptorSets CreateSamplerDescriptorSets(
		const Pipeline&											  pipeline,
		uint32_t												  set_index,
		const std::vector<Image>&								  images,
		const std::vector<std::reference_wrapper<const Sampler>>& samplers) const;

	DescriptorSets CreateTextureArrayDescriptorSet(
		const Pipeline&											  pipeline,
		uint32_t												  set_index,
		uint32_t												  capacity,
		const std::vector<Image>&								  images,
		const std::vector<std::reference_wrapper<const Sampler>>& samplers,
		const Image&											  fallback_image,
		const Sampler&											  fallback_sampler) const;

	DescriptorSets CreateStorageDescriptorSets(
		const Pipeline&			   pipeline,
		uint32_t				   set_index,
		const std::vector<Buffer>& storage_buffers) const;

	Gui CreateGui(GLFWwindow* window, const SwapChain& swapchain, uint32_t frames_in_flight) const;

	/** Blocks until the device has completed all previously submitted work. */
	void WaitForCommands() const;

private:
	/** Owning pointer to the Vulkan instance/device/queues. */
	std::unique_ptr<Backend::VulkanContext> vulkanContextPtr;
};

} // namespace Ping