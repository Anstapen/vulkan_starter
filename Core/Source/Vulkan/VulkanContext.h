#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <GLFW/glfw3.h>

namespace Backend
{
	class VKManager;
	class VulkanContext
	{
		friend class VKManager;
	public:
		virtual ~VulkanContext() = default;
		VulkanContext(const VulkanContext& other) = delete;
		VulkanContext& operator=(const VulkanContext& other) = delete;
		VulkanContext(VulkanContext&& other) noexcept;
		VulkanContext& operator=(VulkanContext&& other) noexcept;
	public:

		vk::raii::Instance instance;
		//vk::raii::PhysicalDevice physicalDevice;
		vk::raii::Device device;
		vk::raii::DebugUtilsMessengerEXT debug_messenger;
		//vk::raii::Queue graphicsQueue;
		//vk::raii::Queue computeQueue;
		//vk::raii::Queue transferQueue;
	private:
		VulkanContext();
		VulkanContext(vk::raii::Instance&& in_instance, vk::raii::Device&& in_device, vk::raii::DebugUtilsMessengerEXT in_debug_messenger);
	};
}

