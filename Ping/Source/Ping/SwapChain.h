#pragma once
#include <cstdint>
#include <memory>
#include <utility>

/* forward declaration to avoid pulling GLFW headers into this public-facing header */
struct GLFWwindow;

namespace Backend
{
class VulkanSwapChain;
class VKManager;
} // namespace Backend

namespace Ping
{
class Device;
class CommandBuffer;

/**
 * An RAII-owned swapchain, created via `Device::CreateSwapChain`.
 *
 * @note Move-only: owns the backend swapchain and its per-frame synchronization objects for its
 * lifetime. Must not outlive the `Device` it was created from.
 */
class SwapChain
{
	friend class Device;
	friend class CommandBuffer;

public:
	/** Takes ownership of an existing backend swapchain. Used internally by `Device::CreateSwapChain`. */
	SwapChain(Backend::VulkanSwapChain&& in_swapChain);
	~SwapChain();
	SwapChain(const SwapChain& other) = delete;
	/** Move-constructs from `other`, taking over its backend swapchain. */
	SwapChain(SwapChain&& other);
	SwapChain& operator=(const SwapChain& other) = delete;
	/** Move-assigns from `other`, taking over its backend swapchain. */
	SwapChain& operator=(SwapChain&& other);

public:
	/**
	 * Acquires the next presentable image for the given frame-in-flight slot.
	 *
	 * @param frameIndex Index of the frame-in-flight slot to acquire for.
	 * @return The acquired image index, or `std::numeric_limits<uint32_t>::max()` if the swapchain
	 * is out of date and must be recreated via `Recreate` before rendering can continue.
	 */
	uint32_t AcquireNextImage(uint32_t frameIndex);

	/**
	 * Presents the image at `image_index` to the screen.
	 *
	 * @return `false` if the swapchain is suboptimal/out of date and must be recreated via `Recreate`,
	 * `true` otherwise. The return value must not be discarded.
	 */
	[[nodiscard]] bool Present(const Device& device, uint32_t image_index);

	/**
	 * Rebuilds the swapchain against `window`'s current size (e.g. after a resize or `Present` failure).
	 * Blocks (pumping window events) while the framebuffer size is zero, e.g. while minimized.
	 */
	void Recreate(const Device& device, GLFWwindow* window, uint32_t frames_in_flight);

	std::pair<uint32_t, uint32_t> GetExtent() const;

private:
	/** Owning pointer to the backend swapchain. */
	std::unique_ptr<Backend::VulkanSwapChain> vulkanSwapChainPtr;
};

} // namespace Ping
