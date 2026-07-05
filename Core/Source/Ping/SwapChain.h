#pragma once
#include <memory>
#include <cstdint>

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
		uint32_t AcquireNextImage();
		void Present(const Device& device, uint32_t image_index);
	private:
		std::unique_ptr<Backend::VulkanSwapChain> vulkanSwapChainPtr;
	};

}
