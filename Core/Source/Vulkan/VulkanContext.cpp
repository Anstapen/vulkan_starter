#include "VulkanContext.h"

using namespace Backend;

VulkanContext::VulkanContext(VulkanContext&& other) noexcept :
	instance(std::move(other.instance)),
	device(std::move(other.device)),
	queues(std::move(other.queues)),
	surface(std::move(other.surface)),
	phys_device(std::move(other.phys_device))
{
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept
{
	instance = std::move(other.instance);
	device = std::move(other.device);
	queues = std::move(other.queues);
	surface = std::move(other.surface);
	phys_device = std::move(other.phys_device);
	return *this;
}

VulkanContext::VulkanContext()
	: instance(nullptr), device(nullptr), queues(), surface(nullptr), phys_device(nullptr)
{
}

VulkanContext::VulkanContext(vk::raii::Instance&& in_instance, vk::raii::PhysicalDevice in_phys_device, vk::raii::Device&& in_device, std::vector<vk::raii::Queue> in_queues, vk::raii::SurfaceKHR in_surface) :
	instance(std::move(in_instance)),
	phys_device(std::move(in_phys_device)),
	device(std::move(in_device)),
	queues(std::move(in_queues)),
	surface(std::move(in_surface))
{
}
