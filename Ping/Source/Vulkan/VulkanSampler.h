#pragma once
#include "VulkanCommon.h"

namespace Backend
{
class VKManager;

/**
 * RAII-owning backend counterpart of `Ping::Sampler`: a `vk::raii::Sampler` describing how an
 * image is sampled by shaders (filtering, addressing, mipmapping).
 *
 * @note Move-only. Built by `VKManager::CreateSampler`; not intended to be constructed directly
 * elsewhere.
 */
class VulkanSampler
{
	friend class VKManager;

public:
	/** Takes ownership of an already-created sampler. */
	VulkanSampler(vk::raii::Sampler&& in_sampler) noexcept;
	~VulkanSampler();
	VulkanSampler(const VulkanSampler& other) = delete;
	/** Move-constructs from `other`, taking over its sampler. */
	VulkanSampler(VulkanSampler&& other) noexcept;
	VulkanSampler& operator=(const VulkanSampler& other) = delete;
	/** Move-assigns from `other`, taking over its sampler. */
	VulkanSampler& operator=(VulkanSampler&& other) noexcept;

private:
	/** The underlying Vulkan sampler. */
	vk::raii::Sampler sampler;
};

} // namespace Backend
