#pragma once
#include "Ping/Types.h"
#include "VulkanCommon.h"
#include "VulkanContext.h"

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
	VulkanBuffer(
		vk::raii::Buffer&&	   in_buffer,
		vk::raii::DeviceMemory in_memory,
		void*				   in_data,
		uint64_t			   size,
		Ping::BufferUsage	   in_buffer_usage,
		Ping::MemoryProperty   in_memory_property) noexcept;
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
	 * Resize the buffer to \a new_size.
	 *
	 * @warning If \a new_size that is smaller than the current size, the function returns immediately!
	 *
	 * @warning The function currently throws when an attempt to resize a device-local buffer is made!
	 */
	void Resize(const VulkanContext& context, uint64_t new_size);

	/**
	 * @warning Only valid if the buffer's memory is host-visible (i.e. `data` is non-null).
	 * @warning The returned pointer is unvalidated once a Resize() operation happens.
	 * @return Pointer to the start of the mapped memory.
	 */
	void* GetMappedPtr();

	/**
	 * Copies the given host memory into the device local GPU buffer.
	 *
	 * @warning This function will throw an error if the buffer is not device local!
	 */
	void CopyHostData(const VulkanContext& context, const void* src, uint64_t size);

private:
	/** The underlying Vulkan buffer. */
	vk::raii::Buffer buffer;
	/** Device memory bound to `buffer`. */
	vk::raii::DeviceMemory memory;
	/** The buffer usage type used to create that buffer. */
	Ping::BufferUsage bufferUsage;
	/** The memory property type used to allocate the underlying memory. */
	Ping::MemoryProperty memoryProperty;
	/** Mapped pointer into `memory`, or `nullptr` if not host-visible. */
	void* data;
	/** Size of the buffer in bytes. */
	uint64_t size;
};
} // namespace Backend