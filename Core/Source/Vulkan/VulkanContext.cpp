#include "VulkanContext.h"

using namespace Backend;

VulkanContext::VulkanContext(VulkanContext&& other) noexcept :
	instance(std::move(other.instance)),
	device(std::move(other.device)),
	debug_messenger(std::move(other.debug_messenger))
{
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept
{
	instance = std::move(other.instance);
	device = std::move(other.device);
	debug_messenger = std::move(other.debug_messenger);
	return *this;
}

VulkanContext::VulkanContext()
	: instance(nullptr), device(nullptr), debug_messenger(nullptr)
{
}

VulkanContext::VulkanContext(vk::raii::Instance&& in_instance, vk::raii::Device&& in_device, vk::raii::DebugUtilsMessengerEXT in_debug_messenger) :
	instance(std::move(in_instance)),
	device(std::move(in_device)),
	debug_messenger(std::move(in_debug_messenger))
{
}
