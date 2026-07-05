#include "VulkanSwapChain.h"
#include "VulkanCommon.h"
#include "VKManager.h"

using namespace Backend;

Backend::VulkanSwapChain::VulkanSwapChain(
	const vk::raii::Device& device,
	vk::raii::SwapchainKHR&& in_swapChain,
	vk::SurfaceFormatKHR in_swapChainSurfaceFormat,
	vk::Extent2D in_swapChainExtent) :
	swapChain(std::move(in_swapChain)),
	swapChainImages(swapChain.getImages()),
	swapChainImageViews(),
	swapChainSurfaceFormat(in_swapChainSurfaceFormat),
	swapChainExtent(in_swapChainExtent),
	presentCompleteSemaphore(device, vk::SemaphoreCreateInfo()),
	renderFinishedSemaphore(device, vk::SemaphoreCreateInfo())
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
	swapChainExtent(other.swapChainExtent),
	presentCompleteSemaphore(std::move(other.presentCompleteSemaphore)),
	renderFinishedSemaphore(std::move(other.renderFinishedSemaphore))
{
}

VulkanSwapChain& Backend::VulkanSwapChain::operator=(VulkanSwapChain&& other) noexcept
{
	swapChain = std::move(other.swapChain);
	swapChainImages = std::move(other.swapChainImages);
	swapChainImageViews = std::move(other.swapChainImageViews);
	swapChainSurfaceFormat = other.swapChainSurfaceFormat;
	swapChainExtent = other.swapChainExtent;
	presentCompleteSemaphore = std::move(other.presentCompleteSemaphore);
	renderFinishedSemaphore = std::move(other.renderFinishedSemaphore);
	return *this;
}

uint32_t Backend::VulkanSwapChain::AcquireNextImage() const
{
	auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);

	return imageIndex;
}

void Backend::VulkanSwapChain::Present(VulkanContext& context, uint32_t image_index)
{
	const vk::PresentInfoKHR presentInfoKHR{
	.waitSemaphoreCount = 1,
	.pWaitSemaphores = &*renderFinishedSemaphore,
	.swapchainCount = 1,
	.pSwapchains = &*swapChain,
	.pImageIndices = &image_index };

	uint32_t q_index = VKManager::GetQueueIndex(context, Ping::QueueType::Graphics);

	context.queues[q_index].queue.presentKHR(presentInfoKHR);
}
