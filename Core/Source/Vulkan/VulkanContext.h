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
		vk::raii::PhysicalDevice phys_device;
		vk::raii::SurfaceKHR surface;
		vk::raii::Device device;
		std::vector<vk::raii::Queue> queues;
	private:
		VulkanContext();
		VulkanContext(vk::raii::Instance&& in_instance, vk::raii::PhysicalDevice in_phys_device, vk::raii::Device&& in_device, std::vector<vk::raii::Queue> in_queues, vk::raii::SurfaceKHR in_surface);
	};
}

