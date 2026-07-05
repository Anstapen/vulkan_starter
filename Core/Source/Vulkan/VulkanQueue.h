#pragma once
#include "VulkanCommon.h"
#include "Ping/Types.h"
#include <vector>

namespace Backend {
	class VulkanQueue
	{
	public:
		VulkanQueue() noexcept;
		VulkanQueue(Ping::QueueType in_type, vk::raii::Queue&& in_queue) noexcept;
		~VulkanQueue() noexcept = default;
		VulkanQueue(const VulkanQueue& other) = delete;
		VulkanQueue(VulkanQueue&& other) noexcept;
		VulkanQueue& operator=(const VulkanQueue& other) = delete;
		VulkanQueue& operator=(VulkanQueue&& other) noexcept;
		Ping::QueueType type;
		vk::raii::Queue queue;
	};

	typedef std::vector<VulkanQueue> VulkanQueues;
}