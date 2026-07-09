#pragma once
#include <memory>

namespace Backend
{
class VulkanBuffer;
}

namespace Ping
{
class Device;
class CommandBuffer;

/**
 * An RAII-owned GPU buffer, created via `Device::CreateBuffer`.
 *
 * @note Move-only: a `Buffer` owns the backing device memory and buffer object for its lifetime and
 * releases them on destruction, so it must not be used after being moved from.
 */
class Buffer
{
	friend class Device;
	friend class CommandBuffer;

public:
	/** Takes ownership of an existing backend buffer. Used internally by `Device::CreateBuffer`. */
	Buffer(Backend::VulkanBuffer&& in_buffer) noexcept;
	~Buffer();
	Buffer(const Buffer& other) = delete;
	/** Move-constructs from `other`, taking over its backend buffer. */
	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(const Buffer& other) = delete;
	/** Move-assigns from `other`, taking over its backend buffer. */
	Buffer& operator=(Buffer&& other) noexcept;

public:
	/** Size of the buffer in bytes, as requested via `Device::CreateBuffer`. */
	uint64_t Size() const;

	/**
	 * Resize the buffer to \a new_size.
	 * 
	 * @warning The buffer needs to have the BufferUsage::TransferSrc flag!
	 */
	void Resize(const Device& device, uint64_t new_size);

	/**
	 * Returns a pointer to the buffer's persistently-mapped host memory.
	 * @return Pointer to the start of the mapped memory.
	 */
	void* GetMappedPtr();

	/**
	* Copy given host data to a device-local GPU buffer using an intermediate staging buffer.
	* 
	* @warning This function will throw an error if it is used on a buffer that is not device local!
	*/
	void CopyHostData(const Device& device, const void* src, uint64_t size);

private:
	/** Owning pointer to the backend buffer and its memory. */
	std::unique_ptr<Backend::VulkanBuffer> vulkanBufferPtr;
};
} // namespace Ping
