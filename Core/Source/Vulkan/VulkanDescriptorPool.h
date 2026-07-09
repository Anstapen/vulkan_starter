#pragma once
#include "VulkanCommon.h"
#include <vector>

namespace Backend
{
class VulkanCommandBuffer;

/**
 * RAII-owning backend counterpart of `Ping::DescriptorSets`: a descriptor pool plus the descriptor
 * sets allocated from it.
 *
 * @note Move-only. Built by `VKManager::CreateDescriptorSets`; not intended to be constructed
 * directly elsewhere.
 *
 * @warning `pool` must be created with `vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet` set.
 * Each `vk::raii::DescriptorSet` in `sets` calls `vkFreeDescriptorSets` from its own destructor
 * unconditionally; without that flag on the pool, that call is invalid usage. `pool` is declared
 * before `sets` so `sets` (destroyed first, since members are destroyed in reverse declaration
 * order) still has a live pool to free into.
 */
class VulkanDescriptorPool
{
	friend class VulkanCommandBuffer;

public:
	/** Takes ownership of an already-created descriptor pool and the sets allocated from it. */
	VulkanDescriptorPool(vk::raii::DescriptorPool&& in_pool, std::vector<vk::raii::DescriptorSet>&& in_sets) noexcept;
	VulkanDescriptorPool(const VulkanDescriptorPool& other) = delete;
	/** Move-constructs from `other`, taking over its descriptor pool and sets. */
	VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept;
	VulkanDescriptorPool& operator=(const VulkanDescriptorPool& other) = delete;
	/** Move-assigns from `other`, taking over its descriptor pool and sets. */
	VulkanDescriptorPool& operator=(VulkanDescriptorPool&& other) noexcept;
	~VulkanDescriptorPool() = default;

private:
	/** The pool `sets` were allocated from. Declared before `sets`: see the class-level @warning. */
	vk::raii::DescriptorPool pool;
	/** One descriptor set per frame in flight, in the same order as the buffers passed to
	 * `VKManager::CreateDescriptorSets`. */
	std::vector<vk::raii::DescriptorSet> sets;
};

} // namespace Backend
