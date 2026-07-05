#include "VulkanSwapChain.h"
#include "VulkanCommon.h"
#include "VKManager.h"

using namespace Backend;

Backend::VulkanSwapChain::VulkanSwapChain(
	const vk::raii::Device& device,
	vk::raii::SwapchainKHR&& in_swapChain,
	vk::SurfaceFormatKHR in_swapChainSurfaceFormat,
	vk::Extent2D in_swapChainExtent,
	uint32_t frames_in_flight) :
	swapChain(std::move(in_swapChain)),
	swapChainImages(swapChain.getImages()),
	swapChainImageViews(),
	swapChainSurfaceFormat(in_swapChainSurfaceFormat),
	swapChainExtent(in_swapChainExtent),
	presentCompleteSemaphores(),
	renderFinishedSemaphores()
{
	vk::ImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = swapChainSurfaceFormat.format;
	imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

	for (auto& image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
		renderFinishedSemaphores.emplace_back(vk::raii::Semaphore(device, vk::SemaphoreCreateInfo()));
	}

	for (uint32_t i = 0; i < frames_in_flight; i++)
	{
		presentCompleteSemaphores.emplace_back(vk::raii::Semaphore(device, vk::SemaphoreCreateInfo()));
	}
}

Backend::VulkanSwapChain::VulkanSwapChain(VulkanSwapChain&& other) noexcept :
	swapChain(std::move(other.swapChain)),
	swapChainImages(std::move(other.swapChainImages)),
	swapChainImageViews(std::move(other.swapChainImageViews)),
	swapChainSurfaceFormat(other.swapChainSurfaceFormat),
	swapChainExtent(other.swapChainExtent),
	presentCompleteSemaphores(std::move(other.presentCompleteSemaphores)),
	renderFinishedSemaphores(std::move(other.renderFinishedSemaphores))
{
}

VulkanSwapChain& Backend::VulkanSwapChain::operator=(VulkanSwapChain&& other) noexcept
{
	swapChain = std::move(other.swapChain);
	swapChainImages = std::move(other.swapChainImages);
	swapChainImageViews = std::move(other.swapChainImageViews);
	swapChainSurfaceFormat = other.swapChainSurfaceFormat;
	swapChainExtent = other.swapChainExtent;
	presentCompleteSemaphores = std::move(other.presentCompleteSemaphores);
	renderFinishedSemaphores = std::move(other.renderFinishedSemaphores);
	return *this;
}

uint32_t Backend::VulkanSwapChain::AcquireNextImage(uint32_t frameIndex) const
{
	auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);


	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		return std::numeric_limits<uint32_t>::max();
	}
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	return imageIndex;
}

bool Backend::VulkanSwapChain::Present(VulkanContext& context, uint32_t image_index)
{
	const vk::PresentInfoKHR presentInfoKHR{
	.waitSemaphoreCount = 1,
	.pWaitSemaphores = &*renderFinishedSemaphores[image_index],
	.swapchainCount = 1,
	.pSwapchains = &*swapChain,
	.pImageIndices = &image_index };

	uint32_t q_index = VKManager::GetQueueIndex(context, Ping::QueueType::Graphics);

	vk::Result result = context.queues[q_index].queue.presentKHR(presentInfoKHR);

	return !((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR));
}
