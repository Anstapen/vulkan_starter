#include "DescriptorSets.h"
#include "Vulkan/VulkanDescriptorPool.h"

using namespace Ping;

DescriptorSets::DescriptorSets(Backend::VulkanDescriptorPool&& in_pool) noexcept
	: vulkanDescriptorPoolPtr(std::make_unique<Backend::VulkanDescriptorPool>(std::move(in_pool)))
{
}

Ping::DescriptorSets::DescriptorSets(DescriptorSets&& other) noexcept
	: vulkanDescriptorPoolPtr(std::move(other.vulkanDescriptorPoolPtr))
{
}

DescriptorSets& DescriptorSets::operator=(DescriptorSets&& other) noexcept
{
	vulkanDescriptorPoolPtr = std::move(other.vulkanDescriptorPoolPtr);
	return *this;
}

DescriptorSets::~DescriptorSets() {}
