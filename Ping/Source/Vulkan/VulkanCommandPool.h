#pragma once
#include "Ping/Types.h"
#include "VulkanCommon.h"

#include <cstdint>

namespace Backend
{

/** A Vulkan command pool tagged with which `Ping::QueueType` it allocates command buffers for. */
class VulkanCommandPool
{
public:
	/** Takes ownership of an already-created command pool, tagged as `in_type`. */
	VulkanCommandPool(
		Ping::QueueType			in_type,
		vk::raii::CommandPool&& in_cmd_pool,
		uint32_t				in_queue_family_index) noexcept;
	~VulkanCommandPool() noexcept = default;
	VulkanCommandPool(const VulkanCommandPool& other) = delete;
	/** Move-constructs from `other`, taking over its type and command pool handle. */
	VulkanCommandPool(VulkanCommandPool&& other) noexcept;
	VulkanCommandPool& operator=(const VulkanCommandPool& other) = delete;
	/** Move-assigns from `other`, taking over its type and command pool handle. */
	VulkanCommandPool& operator=(VulkanCommandPool&& other) noexcept;
	/** Which queue family this pool allocates command buffers for. */
	Ping::QueueType type;
	/** The index of the queue family this command pool is related to */
	uint32_t queueFamilyIndex;
	/** The underlying Vulkan command pool. */
	vk::raii::CommandPool commandPool;
};

} // namespace Backend