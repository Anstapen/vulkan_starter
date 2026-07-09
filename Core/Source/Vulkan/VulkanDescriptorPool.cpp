#include "VulkanDescriptorPool.h"

using namespace Backend;

VulkanDescriptorPool::VulkanDescriptorPool(
	vk::raii::DescriptorPool&&			   in_pool,
	std::vector<vk::raii::DescriptorSet>&& in_sets) noexcept
	: pool(std::move(in_pool)), sets(std::move(in_sets))
{
}

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept
	: pool(std::move(other.pool)), sets(std::move(other.sets))
{
}

VulkanDescriptorPool& VulkanDescriptorPool::operator=(VulkanDescriptorPool&& other) noexcept
{
	pool = std::move(other.pool);
	sets = std::move(other.sets);
	return *this;
}
