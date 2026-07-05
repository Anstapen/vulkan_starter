#pragma once
#include "VulkanCommon.h"
#include "Ping/Types.h"

#include <cstdint>

namespace Backend {

	class VulkanCommandPool
	{
	public:
		VulkanCommandPool(Ping::QueueType in_type, vk::raii::CommandPool&& in_cmd_pool) noexcept;
		~VulkanCommandPool() noexcept = default;
		VulkanCommandPool(const VulkanCommandPool& other) = delete;
		VulkanCommandPool(VulkanCommandPool&& other) noexcept;
		VulkanCommandPool& operator=(const VulkanCommandPool& other) = delete;
		VulkanCommandPool& operator=(VulkanCommandPool&& other) noexcept;
		Ping::QueueType type;
		vk::raii::CommandPool commandPool;
	};

}