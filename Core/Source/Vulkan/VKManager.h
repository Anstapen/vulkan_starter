#pragma once
#include "Logger/Logger.h"
#include "VulkanContext.h"
#include "VKUtil.h"
#include "vulkan/vulkan_raii.hpp"
#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include <vector>

namespace Backend {

	class VKManager {
	public:
		static void Init();

		static VulkanContext CreateVulkanContext(const std::vector<VKQueueFamilyProperties>& wanted_queues,
			const std::vector<const char*>& wanted_extensions,
			const std::vector<const char*>& wanted_validation_layers);

	private:
		static vk::raii::Instance
			CreateInstance(const std::vector<const char*>& wanted_extensions,
				const std::vector<const char*>& wanted_validation_layers);
		static vk::raii::Device SelectBestDevice(vk::raii::Instance& instance,
			const std::vector<VKQueueFamilyProperties>& wanted_queues);
		static void AddRequiredExtensions(VKExtensions& extensions);
		static vk::raii::DebugUtilsMessengerEXT SetupDebugCallback(vk::raii::Instance & instance, PFN_vkDebugUtilsMessengerCallbackEXT user_callback);
		static bool IsDeviceSuitable(const vk::raii::PhysicalDevice& device);
		static vk::raii::Device CreateLogicalDevice(
			const vk::raii::PhysicalDevice &phys_device,
			const std::vector<VKQueueFamilyProperties>& wanted_queues,
			const std::vector<const char*>& wanted_extensions);

	private:
		static bool is_initialized;
		static std::unique_ptr<vk::raii::Context> context;
		static Mupfel::Logger::SafeLoggerPtr logger;
		static std::array<char*, 32> extension_array;
	};

}
