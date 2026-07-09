#pragma once
#include <memory>

namespace Backend
{
class VulkanDescriptorPool;
}

namespace Ping
{
class CommandBuffer;
class Device;

/**
 * An RAII-owned descriptor pool plus one descriptor set per frame in flight, created via
 * `Device::CreateDescriptorSets`.
 *
 * Each set's bindings are written once, at creation time, to point at the matching entry of the
 * `Buffer`s passed to `Device::CreateDescriptorSets` — to push new data (e.g. a per-frame MVP
 * matrix), write into that `Buffer`'s mapped memory, not through this class.
 *
 * @note Move-only: owns the backend descriptor pool (and the sets allocated from it) for its
 * lifetime. Must not outlive the `Pipeline` its descriptor set layout was built from, nor the
 * `Buffer`s its sets were written to point at.
 */
class DescriptorSets
{
	friend class CommandBuffer;

public:
	/** Takes ownership of an existing backend descriptor pool. Used internally by `Device::CreateDescriptorSets`. */
	DescriptorSets(Backend::VulkanDescriptorPool&& in_pool) noexcept;
	DescriptorSets(const DescriptorSets& other) = delete;
	/** Move-constructs from `other`, taking over its backend descriptor pool. */
	DescriptorSets(DescriptorSets&& other) noexcept;
	DescriptorSets& operator=(const DescriptorSets& other) = delete;
	/** Move-assigns from `other`, taking over its backend descriptor pool. */
	DescriptorSets& operator=(DescriptorSets&& other) noexcept;
	~DescriptorSets();

private:
	/** Owning pointer to the backend descriptor pool and its allocated sets. */
	std::unique_ptr<Backend::VulkanDescriptorPool> vulkanDescriptorPoolPtr;
};

} // namespace Ping
