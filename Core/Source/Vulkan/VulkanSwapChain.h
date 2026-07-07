#pragma once
#include "VulkanCommon.h"
#include "VulkanContext.h"
#include <cstdint>
#include <memory>

namespace Backend
{
/**
 * RAII-owning backend counterpart of `Ping::SwapChain`: the swapchain itself, its image views, and
 * the per-image/per-frame-in-flight semaphores used to synchronize acquire/render/present.
 *
 * @note Move-only. Built by `VKManager::CreateSwapChain`; not intended to be constructed directly
 * elsewhere.
 */
class VulkanSwapChain
{
public:
	/**
	 * Builds the image views (one per swapchain image) and semaphores (one `renderFinishedSemaphores`
	 * per image, one `presentCompleteSemaphores` per frame in flight) for an already-created `in_swapChain`.
	 */
	VulkanSwapChain(
		const vk::raii::Device&	 device,
		vk::raii::SwapchainKHR&& in_swapChain,
		vk::SurfaceFormatKHR	 in_swapChainSurfaceFormat,
		vk::Extent2D			 in_swapChainExtent,
		uint32_t				 frames_in_flight);
	~VulkanSwapChain() = default;
	VulkanSwapChain(const VulkanSwapChain& other) = delete;
	/** Move-constructs from `other`, taking over its swapchain, views, and semaphores. */
	VulkanSwapChain(VulkanSwapChain&& other) noexcept;
	VulkanSwapChain& operator=(const VulkanSwapChain& other) = delete;
	/** Move-assigns from `other`, taking over its swapchain, views, and semaphores. */
	VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;

public:
	/**
	 * Acquires the next image, signaling `presentCompleteSemaphores[frameIndex]` when it's ready.
	 *
	 * @return The acquired image index, or `std::numeric_limits<uint32_t>::max()` if the swapchain
	 * is out of date (`VK_ERROR_OUT_OF_DATE_KHR`).
	 * @throws std::runtime_error on any other unexpected `vk::Result`.
	 */
	uint32_t AcquireNextImage(uint32_t frameIndex) const;

	/**
	 * Presents `image_index` on the context's graphics queue. Returns `false` if the result is
	 * suboptimal or out of date (caller should recreate the swapchain), `true` otherwise.
	 */
	bool Present(VulkanContext& context, uint32_t image_index);

public:
	/** The underlying Vulkan swapchain. */
	vk::raii::SwapchainKHR swapChain;
	/** Non-owning handles to the swapchain's images. */
	std::vector<vk::Image> swapChainImages;
	/** One 2D color view per entry in `swapChainImages`. */
	std::vector<vk::raii::ImageView> swapChainImageViews;
	/** Format/color-space the swapchain was created with. */
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	/** Pixel dimensions of the swapchain images. */
	vk::Extent2D swapChainExtent;
	std::vector<vk::raii::Semaphore>
		/** One per frame in flight; signaled by `AcquireNextImage`. */
		presentCompleteSemaphores;
	/** One per swapchain image; waited on by `Present`. */
	std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
};
} // namespace Backend