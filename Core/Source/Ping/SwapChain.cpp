#include "SwapChain.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Device.h"

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

uint32_t Ping::SwapChain::AcquireNextImage()
{
	return vulkanSwapChainPtr->AcquireNextImage();
}

void Ping::SwapChain::Present(const Device& device, uint32_t image_index)
{
	vulkanSwapChainPtr->Present(*device.vulkanContextPtr.get(), image_index);
}
