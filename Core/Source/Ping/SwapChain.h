#pragma once
#include <memory>
#include <cstdint>
#include "Window/Window.h"

namespace Backend
{
	class VulkanSwapChain;
	class VKManager;
}

namespace Ping
{
	class Device;
	class CommandBuffer;
	class SwapChain
	{
		friend class Device;
		friend class CommandBuffer;
	public:
		SwapChain(Backend::VulkanSwapChain&& in_swapChain);
		~SwapChain();
		SwapChain(const SwapChain& other) = delete;
		SwapChain(SwapChain&& other);
		SwapChain& operator=(const SwapChain& other) = delete;
		SwapChain& operator=(SwapChain&& other);
	public:
		uint32_t AcquireNextImage(uint32_t frameIndex);
		[[nodiscard]] bool Present(const Device& device, uint32_t image_index);
		void Recreate(const Device& device, const Window &window, uint32_t frames_in_flight);
	private:
		std::unique_ptr<Backend::VulkanSwapChain> vulkanSwapChainPtr;
	};

}
