#pragma once
#include "Logger/Logger.h"
#include "VulkanContext.h"
#include "VKUtil.h"
#include "VulkanCommon.h"
#include <array>
#include <vector>
#include "Window/Window.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanPipeline.h"
#include "Ping/Pipeline.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "Ping/CommandBuffer.h"

namespace Backend {

	class VKManager {
	public:
		static void Init();
		static void Shutdown();

		static VulkanContext CreateVulkanContext(
			const Window&								window,
			const std::vector<VKQueueFamilyProperties>& wanted_queues,
			const std::vector<const char*>&				wanted_extensions,
			const std::vector<const char*>&				wanted_validation_layers);

		static VulkanSwapChain CreateSwapChain(
			const VulkanContext &context,
			const Window		&window);

		static VulkanPipeline CreatePipeline(
			const VulkanContext&				context,
			const Ping::PipelineSpecification&	specification,
			const VulkanSwapChain&				swapchain);

		static VulkanCommandBuffers CreateCommandBuffers(
			const VulkanContext&	context,
			Ping::QueueType			type,
			uint32_t				num_buffers);

		static void transitionImageLayout(
			VulkanCommandBuffer					&cmd_buffer,
			VulkanSwapChain						&swapchain,
			uint32_t							imageIndex,
			const Ping::ImageLayoutTransition	&layout_transition);

		static void beginRendering(
			VulkanCommandBuffer& cmd_buffer,
			VulkanSwapChain& swapchain,
			uint32_t							imageIndex);

		static uint32_t GetQueueIndex(
			const VulkanContext& context,
			Ping::QueueType wanted_queue_type
		);

		static void WaitForCommands(const vk::raii::Device& device);
	private:
		static vk::raii::Instance
			CreateInstance(const std::vector<const char*>& wanted_extensions,
				const std::vector<const char*>& wanted_validation_layers);
		static vk::raii::SurfaceKHR CreateSurface(vk::raii::Instance& instance, const Window& window);
		static vk::raii::PhysicalDevice SelectBestDevice(vk::raii::Instance& instance,
			vk::raii::SurfaceKHR& surface);
		static void AddRequiredExtensions(VKExtensions& extensions);
		static void CreateCommandPools(const vk::raii::Device &device,
			const vk::raii::PhysicalDevice phys_device,
			const std::vector<VKQueueFamilyProperties>& wanted_queues,
			std::vector<VulkanCommandPool>& command_pools);
		static vk::raii::DebugUtilsMessengerEXT SetupDebugCallback(vk::raii::Instance& instance, PFN_vkDebugUtilsMessengerCallbackEXT user_callback);
		static bool IsDeviceSuitable(const vk::raii::PhysicalDevice& device);
		static vk::raii::Device CreateLogicalDevice(const vk::raii::PhysicalDevice& phys_device,
			VulkanQueues& queues,
			const std::vector<VKQueueFamilyProperties>& wanted_queues,
			const std::vector<const char*>& wanted_extensions,
			vk::raii::SurfaceKHR& surface);
		static vk::SurfaceFormatKHR SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		static vk::PresentModeKHR SelectPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
		static vk::Extent2D SelectSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window);
		static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
		static std::vector<char> readFile(const std::string& filename);
	private:
		static bool is_initialized;
		static std::unique_ptr<vk::raii::Context> context;
		static Mupfel::Logger::SafeLoggerPtr logger;
		static std::array<char*, 32> extension_array;
	};

}
