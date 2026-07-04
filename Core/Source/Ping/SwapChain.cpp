#include "SwapChain.h"
#include "Vulkan/VulkanSwapChain.h"

using namespace Ping;

SwapChain::SwapChain(Backend::VulkanSwapChain&& in_swapChain) : vulkanSwapChainPtr(std::make_unique<Backend::VulkanSwapChain>(std::move(in_swapChain)))
{
}

SwapChain::~SwapChain()
{
}

SwapChain::SwapChain(SwapChain&& other) : vulkanSwapChainPtr(std::move(other.vulkanSwapChainPtr))
{
}

SwapChain& SwapChain::operator=(SwapChain&& other)
{
	vulkanSwapChainPtr = std::move(other.vulkanSwapChainPtr);
	return *this;
}
