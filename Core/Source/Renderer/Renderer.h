#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

namespace Mupfel {

	class Renderer
	{
	public:
		void Init();
		void Render();
		void DeInit();
	private:
		void InitWindow();
		void InitVulkan();
		void createInstance();
		void pickPhysicalDevice();
		bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
		void createLogicalDevice();
		void createSurface();
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
		vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
		vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
		uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
		void createSwapChain();
		void createImageViews();

	private:
		GLFWwindow *window = nullptr;

		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		vk::raii::Device device = nullptr;
		vk::raii::Queue queue = nullptr;
		vk::raii::SurfaceKHR surface = nullptr;

		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages;
		vk::SurfaceFormatKHR   swapChainSurfaceFormat;
		vk::Extent2D           swapChainExtent;
		std::vector<vk::raii::ImageView> swapChainImageViews;

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


