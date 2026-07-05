#pragma once
#include <memory>
#include <cstdint>
#include "VulkanCommon.h"
#include "VulkanContext.h"

namespace Backend {
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(
			const vk::raii::Device &device,
			vk::raii::SwapchainKHR&& in_swapChain,
			vk::SurfaceFormatKHR in_swapChainSurfaceFormat,
			vk::Extent2D in_swapChainExtent,
			uint32_t frames_in_flight);
		~VulkanSwapChain() = default;
		VulkanSwapChain(const VulkanSwapChain& other) = delete;
		VulkanSwapChain(VulkanSwapChain&& other) noexcept;
		VulkanSwapChain& operator=(const VulkanSwapChain& other) = delete;
		VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;
	public:
		uint32_t AcquireNextImage(uint32_t frameIndex) const;
		bool Present(VulkanContext &context, uint32_t image_index);
	public:
		vk::raii::SwapchainKHR				swapChain;
		std::vector<vk::Image>				swapChainImages;
		std::vector<vk::raii::ImageView>	swapChainImageViews;
		vk::SurfaceFormatKHR				swapChainSurfaceFormat;
		vk::Extent2D						swapChainExtent;
		std::vector<vk::raii::Semaphore>	presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore>	renderFinishedSemaphores;
	};
}