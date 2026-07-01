#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "Logger/Logger.h"

namespace Mupfel {

	class Renderer
	{
	public:
		void Init(VkInstance in_instance);
		void Render();
		void DeInit();
	private:
		void InitVulkan();
		bool isDeviceSuitable(vk::raii::PhysicalDevice const& in_physicalDevice);
		void createLogicalDevice();
		void createSurface();
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
		vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
		vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
		uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
		void createSwapChain();
		void createImageViews();
		bool checkIfExtensionIsSupported(const char * const extension);
		bool checkIfRequiredLayersAreSupported(const std::vector<vk::LayerProperties> &layers);
		void createGrahpicsPipeline();
		vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
		void createCommandPool();
		void createCommandBuffer();
		void transitionImageLayout(
			uint32_t                imageIndex,
			vk::ImageLayout         old_layout,
			vk::ImageLayout         new_layout,
			vk::AccessFlags2        src_access_mask,
			vk::AccessFlags2        dst_access_mask,
			vk::PipelineStageFlags2 src_stage_mask,
			vk::PipelineStageFlags2 dst_stage_mask);
		void recordCommandBuffer(uint32_t imageIndex);
		void drawFrame();
		void createSyncObjects();
		void printQueueFlags(const vk::QueueFamilyProperties& prop);

	private:

		Logger::SafeLoggerPtr logger;

		VkInstance instance = nullptr;

		vk::raii::Context context;
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		vk::raii::Device device = nullptr;
		vk::raii::Queue queue = nullptr;
		vk::raii::SurfaceKHR surface = nullptr;

		uint32_t queueIndex = 0;

		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages;
		vk::SurfaceFormatKHR   swapChainSurfaceFormat;
		vk::Extent2D           swapChainExtent;
		std::vector<vk::raii::ImageView> swapChainImageViews;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline graphicsPipeline = nullptr;
		vk::raii::CommandPool commandPool = nullptr;
		vk::raii::CommandBuffer commandBuffer = nullptr;

		vk::raii::Semaphore presentCompleteSemaphore = nullptr;
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
		vk::raii::Fence drawFence = nullptr;

		const std::vector<char const*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
	};

}


