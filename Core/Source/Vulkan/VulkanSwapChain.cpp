#include "VulkanSwapChain.h"
#include "vulkan/vulkan_raii.hpp"

using namespace Backend;

Backend::VulkanSwapChain::VulkanSwapChain(
	const vk::raii::Device& device,
	vk::raii::SwapchainKHR&& in_swapChain,
	vk::SurfaceFormatKHR in_swapChainSurfaceFormat,
	vk::Extent2D in_swapChainExtent) : swapChain(std::move(in_swapChain)), swapChainImages(swapChain.getImages()), swapChainImageViews(), swapChainSurfaceFormat(in_swapChainSurfaceFormat), swapChainExtent(in_swapChainExtent)
{
	vk::ImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = swapChainSurfaceFormat.format;
	imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

	for (auto& image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
}

Backend::VulkanSwapChain::VulkanSwapChain(VulkanSwapChain&& other) noexcept :
	swapChain(std::move(other.swapChain)),
	swapChainImages(std::move(other.swapChainImages)),
	swapChainImageViews(std::move(other.swapChainImageViews)),
	swapChainSurfaceFormat(other.swapChainSurfaceFormat),
	swapChainExtent(other.swapChainExtent)
{
}

VulkanSwapChain& Backend::VulkanSwapChain::operator=(VulkanSwapChain&& other) noexcept
{
	swapChain = std::move(other.swapChain);
	swapChainImages = std::move(other.swapChainImages);
	swapChainImageViews = std::move(other.swapChainImageViews);
	swapChainSurfaceFormat = other.swapChainSurfaceFormat;
	swapChainExtent = other.swapChainExtent;
	return *this;
}