#include "VulkanCommandPool.h"

using namespace Backend;

Backend::VulkanCommandPool::VulkanCommandPool(
	Ping::QueueType			in_type,
	vk::raii::CommandPool&& in_cmd_pool,
	uint32_t				in_queue_family_index) noexcept
	: type(in_type), commandPool(std::move(in_cmd_pool)), queueFamilyIndex(in_queue_family_index)
{
}

Backend::VulkanCommandPool::VulkanCommandPool(VulkanCommandPool&& other) noexcept
	: type(other.type), commandPool(std::move(other.commandPool)), queueFamilyIndex(other.queueFamilyIndex)
{
}

VulkanCommandPool& Backend::VulkanCommandPool::operator=(VulkanCommandPool&& other) noexcept
{
	type = other.type;
	commandPool = std::move(other.commandPool);
	queueFamilyIndex = other.queueFamilyIndex;
	return *this;
}
