#include "SwapChain.h"
#include "Device.h"
#include "Vulkan/VKManager.h"
#include "Vulkan/VulkanSwapChain.h"

using namespace Ping;

SwapChain::SwapChain(Backend::VulkanSwapChain&& in_swapChain)
	: vulkanSwapChainPtr(std::make_unique<Backend::VulkanSwapChain>(std::move(in_swapChain)))
{
}

SwapChain::~SwapChain() {}

SwapChain::SwapChain(SwapChain&& other) : vulkanSwapChainPtr(std::move(other.vulkanSwapChainPtr)) {}

SwapChain& SwapChain::operator=(SwapChain&& other)
{
	vulkanSwapChainPtr = std::move(other.vulkanSwapChainPtr);
	return *this;
}

uint32_t Ping::SwapChain::AcquireNextImage(uint32_t frameIndex)
{
	return vulkanSwapChainPtr->AcquireNextImage(frameIndex);
}

[[nodiscard]] bool Ping::SwapChain::Present(const Device& device, uint32_t image_index)
{
	return vulkanSwapChainPtr->Present(*device.vulkanContextPtr.get(), image_index);
}

void Ping::SwapChain::Recreate(const Device& device, const Window& window, uint32_t frames_in_flight)
{
	int32_t width = 0, height = 0;
	window.GetFramebufferSize(width, height);
	while (width == 0 || height == 0)
	{
		window.GetFramebufferSize(width, height);
		window.waitEvents();
	}

	device.WaitForCommands();
	vulkanSwapChainPtr.reset();
	vulkanSwapChainPtr = std::make_unique<Backend::VulkanSwapChain>(
		Backend::VKManager::CreateSwapChain(*device.vulkanContextPtr.get(), window, frames_in_flight));
}

std::tuple<uint32_t, uint32_t> Ping::SwapChain::GetExtent() const
{
	return std::tuple<uint32_t, uint32_t>(
		vulkanSwapChainPtr->swapChainExtent.width, vulkanSwapChainPtr->swapChainExtent.height);
}
