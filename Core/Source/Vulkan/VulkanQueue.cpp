#include "VulkanQueue.h"

using namespace Backend;

VulkanQueue::VulkanQueue() noexcept : queue(nullptr), type(Ping::QueueType::None), familyIndex(0) {}

VulkanQueue::VulkanQueue(Ping::QueueType in_type, vk::raii::Queue&& in_queue, uint32_t in_family_index) noexcept
	: queue(std::move(in_queue)), type(in_type), familyIndex(in_family_index)
{
}

Backend::VulkanQueue::VulkanQueue(VulkanQueue&& other) noexcept
	: queue(std::move(other.queue)), type(other.type), familyIndex(other.familyIndex)
{
}

VulkanQueue& Backend::VulkanQueue::operator=(VulkanQueue&& other) noexcept
{
	queue = std::move(other.queue);
	type = other.type;
	familyIndex = other.familyIndex;
	return *this;
}
