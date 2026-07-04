#pragma once
#include <memory>

namespace Backend
{
	class VulkanSwapChain;
}

namespace Ping
{

	class SwapChain
	{
	public:
		SwapChain(Backend::VulkanSwapChain&& in_swapChain);
		~SwapChain();
		SwapChain(const SwapChain& other) = delete;
		SwapChain(SwapChain&& other);
		SwapChain& operator=(const SwapChain& other) = delete;
		SwapChain& operator=(SwapChain&& other);
	private:
		std::unique_ptr<Backend::VulkanSwapChain> vulkanSwapChainPtr;
	};

}
