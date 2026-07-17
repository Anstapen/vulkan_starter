#pragma once
#include "Logger/Logger.h"
#include "Ping/Pipeline.h"
#include "VKUtil.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanCommon.h"
#include "VulkanContext.h"
#include "VulkanDescriptorPool.h"
#include "VulkanGui.h"
#include "VulkanImage.h"
#include "VulkanPipeline.h"
#include "VulkanQueue.h"
#include "VulkanSampler.h"
#include "VulkanSwapChain.h"
#include <array>
#include <optional>
#include <vector>

/* forward declaration to avoid pulling GLFW headers into this public-facing header */
struct GLFWwindow;

namespace Backend
{

/**
 * Static-method class holding all raw Vulkan logic for the backend: instance/device creation,
 * swapchain/pipeline/command-buffer/buffer creation, memory type selection, layout transitions, and
 * dynamic rendering setup. Every `Vulkan*` RAII class is built by (and only by) one of these methods.
 *
 * @note Not instantiable in practice: every member is `static`. `Init` must be called once before
 * any other method (asserted in `CreateVulkanContext`).
 */
class VKManager
{
public:
	/**
	 * Initializes GLFW and the shared `vk::raii::Context`. Idempotent — safe to call more than once
	 * (e.g. once explicitly via `Ping::Init` and again in `Ping::Device`'s constructor).
	 */
	static void Init();

	/** Terminates GLFW. Call once after all `Ping`/backend resources have been destroyed. */
	static void Shutdown();

	/**
	 * Creates the Vulkan instance, surface, physical/logical device, and one queue + command pool
	 * per entry in `wanted_queues`.
	 *
	 * @throws std::runtime_error if no suitable physical device is found, if a requested queue
	 * family isn't available, or if a requested graphics queue lacks presentation support.
	 */
	static VulkanContext CreateVulkanContext(
		GLFWwindow*						   window,
		const std::vector<VKQueueRequest>& wanted_queues,
		const std::vector<const char*>&	   wanted_instance_extensions,
		const std::vector<const char*>&	   wanted_device_extensions,
		const std::vector<const char*>&	   wanted_validation_layers);

	/**
	 * Creates a swapchain sized to `window`'s current framebuffer, preferring `eB8G8R8A8Srgb`/`eSrgbNonlinear`
	 * and `eMailbox` present mode (falling back to the first available format and `eFifo` respectively).
	 */
	static VulkanSwapChain CreateSwapChain(const VulkanContext& context, GLFWwindow* window, uint32_t frames_in_flight);

	/**
	 * Blocks (pumping window events) while `window`'s framebuffer has zero width or height (e.g.
	 * while minimized), writing the first non-zero size observed to `width`/`height`.
	 */
	static void WaitForNonZeroFramebufferSize(GLFWwindow* window, int32_t& width, int32_t& height);

	/**
	 * Builds a graphics pipeline using dynamic rendering (no `vk::RenderPass`) and dynamic
	 * viewport/scissor, loading `specification.shaderFilePath` as a SPIR-V module with `vertMain`/`fragMain`
	 * entry points and alpha blending enabled.
	 */
	static VulkanPipeline CreatePipeline(
		const VulkanContext&			   context,
		const Ping::PipelineSpecification& specification,
		const VulkanSwapChain&			   swapchain);

	/**
	 * Allocates `num_buffers` primary command buffers from `context`'s pool matching `type`, each
	 * paired with a fence created already signaled.
	 */
	static VulkanCommandBuffers
	CreateCommandBuffers(const VulkanContext& context, Ping::QueueType type, uint32_t num_buffers);

	static VulkanDescriptorPool CreateUBODescriptorSets(
		const VulkanContext&					context,
		const VulkanPipeline&					pipeline,
		uint32_t								set_index,
		const std::vector<const VulkanBuffer*>& uniform_buffers);

	static VulkanDescriptorPool CreateSamplerDescriptorSets(
		const VulkanContext&					 context,
		const VulkanPipeline&					 pipeline,
		uint32_t								 set_index,
		const std::vector<const VulkanImage*>&	 images,
		const std::vector<const VulkanSampler*>& samplers);

	static VulkanDescriptorPool CreateTextureArrayDescriptorSet(
		const VulkanContext&					 context,
		const VulkanPipeline&					 pipeline,
		uint32_t								 set_index,
		uint32_t								 capacity,
		const std::vector<const VulkanImage*>&	 images,
		const std::vector<const VulkanSampler*>& samplers,
		const VulkanImage&						 fallback_image,
		const VulkanSampler&					 fallback_sampler);

	static VulkanDescriptorPool CreateStorageDescriptorSets(
		const VulkanContext&					context,
		const VulkanPipeline&					pipeline,
		uint32_t								set_index,
		const std::vector<const VulkanBuffer*>& storage_buffers);

	/**
	 * Creates a buffer of `size` bytes with `usage`, backed by memory satisfying `property`; maps the
	 * memory immediately if `property` includes `MemoryProperty::HostVisible`.
	 */
	static VulkanBuffer CreateBuffer(
		const VulkanContext&	context,
		size_t					size,
		vk::BufferUsageFlags	usage,
		vk::MemoryPropertyFlags property);

	/**
	 * Creates a buffer of `size` bytes with `usage`, backed by memory satisfying `property`; maps the
	 * memory immediately if `property` includes `MemoryProperty::HostVisible`.
	 */
	static VulkanImage CreateImage(
		const VulkanContext&	context,
		uint32_t				width,
		uint32_t				height,
		vk::Format				format,
		vk::ImageTiling			tiling,
		vk::ImageUsageFlags		usage,
		vk::ImageAspectFlags	aspect,
		vk::MemoryPropertyFlags properties);

	/**
	 * Creates a sampler describing how images are sampled by shaders.
	 *
	 * @note Implementation intentionally left to be filled in.
	 */
	static VulkanSampler CreateSampler(const VulkanContext& context, Ping::SamplerSpecification sampler_spec);

	static VulkanGui CreateGui(
		const VulkanContext&   context,
		GLFWwindow*			   window,
		const VulkanSwapChain& swapchain,
		uint32_t			   frames_in_flight);

	static void RenderGui(
		const VulkanContext&	   context,
		const VulkanCommandBuffer& cmd_buffer,
		VulkanGui&				   gui,
		uint32_t				   frame_index);

	/**
	 * Records a `vk::ImageMemoryBarrier2`-based transition of `swapchain`'s image at `imageIndex`, per
	 * `layout_transition`.
	 */
	static void transitionImageLayout(
		VulkanCommandBuffer&			   cmd_buffer,
		VulkanSwapChain&				   swapchain,
		uint32_t						   imageIndex,
		const Ping::ImageLayoutTransition& layout_transition);

	static void transitionImageLayout(
		VulkanCommandBuffer&			   cmd_buffer,
		VulkanImage&					   image,
		const Ping::ImageLayoutTransition& layout_transition);

	static void transitionImageLayout(
		VulkanCommandBuffer&			   cmd_buffer,
		vk::Image						   image,
		const Ping::ImageLayoutTransition& layout_transition);

	/**
	 * Overloaded version for simple image transitions.
	 */
	static void transitionImageLayout(
		VulkanCommandBuffer&   cmd_buffer,
		const vk::raii::Image& image,
		vk::ImageLayout		   old_layout,
		vk::ImageLayout		   new_layout);

	/**
	 * Begins dynamic rendering into `swapchain`'s image view at `imageIndex` (clearing to opaque black)
	 * and sets the viewport/scissor to the swapchain's full extent.
	 */
	static void beginRendering(
		VulkanCommandBuffer& cmd_buffer,
		VulkanSwapChain&	 swapchain,
		VulkanImage&		 depth_buffer,
		uint32_t			 imageIndex);

	/**
	 * @throws std::runtime_error if `context` has no queue of type `wanted_queue_type`.
	 * @return Index into `context.queues` of the first queue matching `wanted_queue_type`.
	 */
	static uint32_t GetQueueIndex(const VulkanContext& context, Ping::QueueType wanted_queue_type);

	/** Blocks until `device` has completed all previously submitted work (`vkDeviceWaitIdle`). */
	static void WaitForCommands(const vk::raii::Device& device);

	static void CopyBuffer(
		const VulkanContext& context,
		vk::raii::Buffer&	 srcBuffer,
		vk::raii::Buffer&	 dstBuffer,
		vk::DeviceSize		 size);

	static std::optional<VulkanImage>
	LoadVulkanImage(const VulkanContext& context, const std::string& path, vk::ImageUsageFlags usage);

private:
	/**
	 * Creates the `vk::Instance`, enabling `wanted_extensions`/`wanted_validation_layers` (each
	 * filtered to what's actually supported) plus whatever `AddRequiredExtensions` adds. In debug
	 * builds (`!NDEBUG`), also enables `VK_EXT_debug_utils` and installs `debugCallback` via
	 * `SetupDebugCallback` the first time an instance is created.
	 */
	static vk::raii::Instance CreateInstance(
		const std::vector<const char*>& wanted_extensions,
		const std::vector<const char*>& wanted_validation_layers);

	/**
	 * Creates the window's presentation surface via `glfwCreateWindowSurface`.
	 * @throws std::runtime_error if GLFW fails to create the surface.
	 */
	static vk::raii::SurfaceKHR CreateSurface(vk::raii::Instance& instance, GLFWwindow* window);

	/**
	 * Picks a physical device via `IsDeviceSuitable`. If multiple devices are suitable, the last one
	 * enumerated is returned — there is currently no ranking beyond having a graphics queue family.
	 * @throws std::runtime_error if no device is suitable.
	 */
	static vk::raii::PhysicalDevice SelectBestDevice(vk::raii::Instance& instance);

	/** Adds the instance extensions GLFW requires for window surface creation on the current platform. */
	static void AddRequiredExtensions(VKExtensions& extensions);

	/**
	 * Creates one command pool per entry in `wanted_queues` (resettable-command-buffer flag set),
	 * tagged with the matching `Ping::QueueType`.
	 * @throws std::runtime_error if a queue family index can't be resolved for one of `wanted_queues`.
	 */
	static void CreateCommandPools(
		const vk::raii::Device&				device,
		const std::vector<VKResolvedQueue>& resolved_queues,
		std::vector<VulkanCommandPool>&		command_pools);

	/**
	 * Installs `user_callback` as a debug messenger for warning/error validation and general/performance
	 * messages. A no-op returning a null handle in release (`NDEBUG`) builds.
	 */
	static vk::raii::DebugUtilsMessengerEXT
	SetupDebugCallback(vk::raii::Instance& instance, vk::PFN_DebugUtilsMessengerCallbackEXT user_callback);

	/** Whether `device` has at least one queue family, and at least one of them supports graphics. */
	static bool IsDeviceSuitable(const vk::raii::PhysicalDevice& device);

	/**
	 * Creates the logical device and one `vk::raii::Queue` per entry in `wanted_queues`, appended to `queues`.
	 * @throws std::runtime_error if a requested queue family can't be found, or a requested graphics
	 * queue doesn't support presentation to `surface`.
	 */
	static vk::raii::Device CreateLogicalDevice(
		const vk::raii::PhysicalDevice&		phys_device,
		VulkanQueues&						queues,
		const std::vector<VKResolvedQueue>& wanted_queues,
		const std::vector<const char*>&		wanted_device_extensions,
		vk::raii::SurfaceKHR&				surface);

	/** Prefers `eB8G8R8A8Srgb`/`eSrgbNonlinear` from `availableFormats`, falling back to the first available format. */
	static vk::SurfaceFormatKHR SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

	/** Prefers `eMailbox` from `availablePresentModes`, falling back to `eFifo` (which must always be available). */
	static vk::PresentModeKHR SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

	/**
	 * Returns `capabilities.currentExtent` if it's well-defined, otherwise clamps `window`'s current
	 * framebuffer size to `capabilities`' min/max extent.
	 */
	static vk::Extent2D SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	/**
	 * Requests at least 3 swapchain images (for `frames_in_flight`-independent triple buffering),
	 * clamped to `surfaceCapabilities`' maximum if it has one.
	 */
	static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);

	/**
	 * Reads the entire binary contents of `filename` (e.g. a compiled SPIR-V shader).
	 * @throws std::runtime_error if the file can't be opened.
	 */
	static std::vector<char> readFile(const std::string& filename);

	/**
	 * Finds a memory type index among `phys_devicee`'s heaps whose bit is set in `type_filter` (a
	 * `VkMemoryRequirements::memoryTypeBits` mask) and which supports all flags in `property`.
	 * @throws std::runtime_error if no matching memory type exists.
	 */
	static uint32_t findMemoryType(
		const vk::raii::PhysicalDevice& phys_devicee,
		uint32_t						type_filter,
		vk::MemoryPropertyFlags			property);

	static std::vector<vk::raii::DescriptorSetLayout>
	CreateDescriptorSetLayouts(const VulkanContext& context, const std::vector<Ping::DescriptorBinding>& bindings);

	static void UploadImageData(
		const VulkanContext& context,
		VulkanImage&		 image,
		const void*			 pixels,
		uint32_t			 width,
		uint32_t			 height,
		uint32_t			 bytes_per_pixel);

private:
	/** Set by `Init`; guards it from running twice. */
	static bool is_initialized;
	/** The shared `vk::raii::Context`, created by `Init`. */
	static std::unique_ptr<vk::raii::Context> vk_context;
	/** Logger created by `Init`, used throughout `VKManager`. */
	static Mupfel::Logger::SafeLoggerPtr logger;
	/** Currently unused. */
	static std::array<char*, 32> extension_array;
};

} // namespace Backend
