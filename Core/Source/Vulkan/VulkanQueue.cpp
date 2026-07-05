#include "VulkanQueue.h"

using namespace Backend;

VulkanQueue::VulkanQueue() noexcept :
	queue(nullptr),
	type(Ping::QueueType::None)
{

}

VulkanQueue::VulkanQueue(Ping::QueueType in_type, vk::raii::Queue&& in_queue) noexcept :
	queue(std::move(in_queue)),
	type(in_type)
{

}

Backend::VulkanQueue::VulkanQueue(VulkanQueue&& other) noexcept :
	queue(std::move(other.queue)),
	type(other.type)
{
}

VulkanQueue& Backend::VulkanQueue::operator=(VulkanQueue&& other) noexcept
{
	queue = std::move(other.queue);
	type = other.type;
	return *this;
}

