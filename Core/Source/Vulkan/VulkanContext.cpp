#include "VulkanContext.h"

using namespace Backend;

VulkanContext::VulkanContext(VulkanContext&& other) noexcept
	: instance(std::move(other.instance)), phys_device(std::move(other.phys_device)), device(std::move(other.device)),
	  queues(std::move(other.queues)), surface(std::move(other.surface)), command_pools(std::move(other.command_pools)),
	  debugMessenger(std::move(other.debugMessenger))
{
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept
{
	instance = std::move(other.instance);
	phys_device = std::move(other.phys_device);
	device = std::move(other.device);
	queues = std::move(other.queues);
	surface = std::move(other.surface);
	command_pools = std::move(other.command_pools);
	debugMessenger = std::move(other.debugMessenger);
	return *this;
}

VulkanContext::VulkanContext()
	: instance(nullptr), device(nullptr), queues(), surface(nullptr), phys_device(nullptr), command_pools(),
	  debugMessenger(nullptr)
{
}

VulkanContext::VulkanContext(
	vk::raii::Instance&&			 in_instance,
	vk::raii::PhysicalDevice		 in_phys_device,
	vk::raii::Device&&				 in_device,
	std::vector<VulkanQueue>		 in_queues,
	vk::raii::SurfaceKHR			 in_surface,
	std::vector<VulkanCommandPool>	 in_command_pools,
	vk::raii::DebugUtilsMessengerEXT in_debug_messenger)
	: instance(std::move(in_instance)), phys_device(std::move(in_phys_device)), device(std::move(in_device)),
	  queues(std::move(in_queues)), surface(std::move(in_surface)), command_pools(std::move(in_command_pools)),
	  debugMessenger(std::move(in_debug_messenger))
{
}
