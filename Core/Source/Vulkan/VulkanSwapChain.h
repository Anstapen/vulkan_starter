#pragma once
#include <memory>
#include "vulkan/vulkan_raii.hpp"

namespace Backend {
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(
			const vk::raii::Device &device,
			vk::raii::SwapchainKHR&& in_swapChain,
			vk::SurfaceFormatKHR in_swapChainSurfaceFormat,
			vk::Extent2D in_swapChainExtent);
		~VulkanSwapChain() = default;
		VulkanSwapChain(const VulkanSwapChain& other) = delete;
		VulkanSwapChain(VulkanSwapChain&& other) noexcept;
		VulkanSwapChain& operator=(const VulkanSwapChain& other) = delete;
		VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;
	public:
		vk::raii::SwapchainKHR swapChain;
		std::vector<vk::Image> swapChainImages;
		std::vector<vk::raii::ImageView> swapChainImageViews;
		vk::SurfaceFormatKHR   swapChainSurfaceFormat;
		vk::Extent2D           swapChainExtent;
	};
}