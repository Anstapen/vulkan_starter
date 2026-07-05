#pragma once
#include "VulkanCommon.h"
#include <memory>
#include "VulkanCommandPool.h"
#include "VulkanQueue.h"

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
		std::vector<VulkanQueue> queues;
		std::vector<VulkanCommandPool> command_pools;
	private:
		VulkanContext();
		VulkanContext(vk::raii::Instance&& in_instance,
			vk::raii::PhysicalDevice in_phys_device,
			vk::raii::Device&& in_device,
			std::vector<VulkanQueue> in_queues,
			vk::raii::SurfaceKHR in_surface,
			std::vector<VulkanCommandPool> in_command_pools);
	};
}

