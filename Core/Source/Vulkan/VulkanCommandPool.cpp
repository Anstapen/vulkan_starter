#include "VulkanCommandPool.h"

using namespace Backend;

Backend::VulkanCommandPool::VulkanCommandPool(Ping::QueueType in_type, vk::raii::CommandPool&& in_cmd_pool) noexcept :
	type(in_type),
	commandPool(std::move(in_cmd_pool))
{
}

Backend::VulkanCommandPool::VulkanCommandPool(VulkanCommandPool&& other) noexcept : type(other.type), commandPool(std::move(other.commandPool))
{
}

VulkanCommandPool& Backend::VulkanCommandPool::operator=(VulkanCommandPool&& other) noexcept
{
	type = other.type;
	commandPool = std::move(other.commandPool);
	return *this;
}
