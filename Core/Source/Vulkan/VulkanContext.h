#pragma once
#include "VulkanCommandPool.h"
#include "VulkanCommon.h"
#include "VulkanQueue.h"
#include <memory>

namespace Backend
{
class VKManager;

/**
 * RAII-owning backend counterpart of `Ping::Device`: the Vulkan instance, physical/logical device,
 * presentation surface, queues, and their command pools — everything a `Ping::Device` needs to hand
 * out swapchains, pipelines, command buffers, and buffers.
 *
 * @note Move-only. Only `VKManager` (via `VKManager::CreateVulkanContext`) can construct one; every
 * other `Ping`/`Backend` resource is created from and must not outlive it.
 */
class VulkanContext
{
	friend class VKManager;

public:
	virtual ~VulkanContext() = default;
	VulkanContext(const VulkanContext& other) = delete;
	VulkanContext& operator=(const VulkanContext& other) = delete;
	/** Move-constructs from `other`, taking over its instance, device, queues, and command pools. */
	VulkanContext(VulkanContext&& other) noexcept;
	/** Move-assigns from `other`, taking over its instance, device, queues, and command pools. */
	VulkanContext& operator=(VulkanContext&& other) noexcept;

public:
	/** The Vulkan instance. */
	vk::raii::Instance instance;
	/** The physical device selected by `VKManager::SelectBestDevice`. */
	vk::raii::PhysicalDevice phys_device;
	/** The logical device. */
	vk::raii::Device device;
	/** Queues created for this device, one per requested queue family. */
	std::vector<VulkanQueue> queues;
	/** Presentation surface for the window this context was created for. */
	vk::raii::SurfaceKHR surface;
	/** Command pools created for this device, one per requested queue family. */
	std::vector<VulkanCommandPool> command_pools;

private:
	VulkanContext();

	/**
	 * Takes ownership of an already-created instance, physical/logical device, queues, surface, and
	 * command pools. Used internally by `VKManager::CreateVulkanContext`.
	 */
	VulkanContext(
		vk::raii::Instance&&		   in_instance,
		vk::raii::PhysicalDevice	   in_phys_device,
		vk::raii::Device&&			   in_device,
		std::vector<VulkanQueue>	   in_queues,
		vk::raii::SurfaceKHR		   in_surface,
		std::vector<VulkanCommandPool> in_command_pools);
};
} // namespace Backend
