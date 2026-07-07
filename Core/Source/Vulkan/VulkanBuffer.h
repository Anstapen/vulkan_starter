#pragma once
#include "Ping/Types.h"
#include "VulkanCommon.h"

namespace Backend
{
class VKManager;
class VulkanCommandBuffer;

/**
 * RAII-owning backend counterpart of `Ping::Buffer`: the buffer and its backing device memory,
 * optionally persistently mapped.
 *
 * @note Move-only. Built by `VKManager::CreateBuffer`; not intended to be constructed directly
 * elsewhere. The destructor unmaps `memory` if it was mapped.
 */
class VulkanBuffer
{
public:
	/** Constructs instances via `VKManager::CreateBuffer`. */
	friend VKManager;
	/** Reads `buffer` to bind it as a vertex buffer. */
	friend VulkanCommandBuffer;

	/**
	 * Takes ownership of an already-created buffer and its bound memory. `in_data` is the mapped
	 * pointer if the memory is host-visible, or `nullptr` if it isn't mapped.
	 */
	VulkanBuffer(vk::raii::Buffer&& in_buffer, vk::raii::DeviceMemory in_memory, void* in_data, uint64_t size) noexcept;
	~VulkanBuffer();
	VulkanBuffer(const VulkanBuffer& other) = delete;
	/** Move-constructs from `other`, taking over its buffer, memory, and mapped pointer. */
	VulkanBuffer(VulkanBuffer&& other) noexcept;
	VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
	/** Move-assigns from `other`, taking over its buffer, memory, and mapped pointer. */
	VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

public:
	/** Size of the buffer in bytes. */
	uint64_t Size() const;

	/**
	 * @warning Only valid if the buffer's memory is host-visible (i.e. `data` is non-null).
	 * @return Pointer to the start of the mapped memory.
	 */
	void* GetMappedPtr();

private:
	/** The underlying Vulkan buffer. */
	vk::raii::Buffer buffer;
	/** Device memory bound to `buffer`. */
	vk::raii::DeviceMemory memory;
	/** Mapped pointer into `memory`, or `nullptr` if not host-visible. */
	void* data;
	/** Size of the buffer in bytes. */
	uint64_t size;
};
} // namespace Backend