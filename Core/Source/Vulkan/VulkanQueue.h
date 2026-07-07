#pragma once
#include "Ping/Types.h"
#include "VulkanCommon.h"
#include <vector>

namespace Backend
{
/**
 * A Vulkan queue tagged with which `Ping::QueueType` it was created to satisfy, so `VKManager::GetQueueIndex`
 * can find the right one later.
 */
class VulkanQueue
{
public:
	/** Constructs an empty (null-handle, `QueueType::None`) queue; not usable until move-assigned a real one. */
	VulkanQueue() noexcept;
	/** Takes ownership of an already-created queue, tagged as `in_type`. */
	VulkanQueue(Ping::QueueType in_type, vk::raii::Queue&& in_queue) noexcept;
	~VulkanQueue() noexcept = default;
	VulkanQueue(const VulkanQueue& other) = delete;
	/** Move-constructs from `other`, taking over its type and queue handle. */
	VulkanQueue(VulkanQueue&& other) noexcept;
	VulkanQueue& operator=(const VulkanQueue& other) = delete;
	/** Move-assigns from `other`, taking over its type and queue handle. */
	VulkanQueue& operator=(VulkanQueue&& other) noexcept;
	/** Which queue type this queue was created to satisfy. */
	Ping::QueueType type;
	/** The underlying Vulkan queue. */
	vk::raii::Queue queue;
};

/** All queues created for a `VulkanContext`, one per requested `VKQueueFamilyProperties` entry. */
typedef std::vector<VulkanQueue> VulkanQueues;
} // namespace Backend