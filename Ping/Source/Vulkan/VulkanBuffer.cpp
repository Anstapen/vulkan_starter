#include "VulkanBuffer.h"
#include "VKManager.h"

using namespace Backend;

Backend::VulkanBuffer::VulkanBuffer(
	vk::raii::Buffer&&		in_buffer,
	vk::raii::DeviceMemory	in_memory,
	void*					in_data,
	uint64_t				in_size,
	vk::BufferUsageFlags	in_buffer_usage,
	vk::MemoryPropertyFlags in_memory_property) noexcept
	: buffer(std::move(in_buffer)), memory(std::move(in_memory)), data(in_data), size(in_size),
	  bufferUsage(in_buffer_usage), memoryProperty(in_memory_property)
{
}

Backend::VulkanBuffer::~VulkanBuffer()
{
	if (data)
	{
		memory.unmapMemory();
	}
}

Backend::VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
	: buffer(std::move(other.buffer)), memory(std::move(other.memory)), data(other.data), size(other.size),
	  bufferUsage(other.bufferUsage), memoryProperty(other.memoryProperty)
{
	other.data = 0;
	other.size = 0;
}

VulkanBuffer& Backend::VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
{
	buffer = std::move(other.buffer);
	memory = std::move(other.memory);
	data = other.data;
	other.data = 0;
	size = other.size;
	other.size = 0;
	bufferUsage = other.bufferUsage;
	memoryProperty = other.memoryProperty;
	return *this;
}

uint64_t Backend::VulkanBuffer::Size() const { return size; }

void Backend::VulkanBuffer::Resize(const VulkanContext& context, uint64_t new_size)
{
	if (new_size <= this->size)
	{
		/* We do not allow shrinking a buffer */
		return;
	}
	/*
	 * Distinguish between two scenarios:
	 * 1. buffer is host visible.
	 * 2. buffer is device local.
	 */
	if (data)
	{
		/* buffer is host visible */
		VulkanBuffer new_buffer = VKManager::CreateBuffer(context, new_size, bufferUsage, memoryProperty);

		/* We asserted that the new buffer size is greater than the current one. */
		std::memcpy(new_buffer.GetMappedPtr(), this->data, this->size);

		/* Unmap the old memory, it is no longer needed. */
		this->memory.unmapMemory();

		/* Assign the new values */
		*this = std::move(new_buffer);
	}
	else
	{
		/* Resizing a device local buffer needs TransferSrc */
		if ((bufferUsage & vk::BufferUsageFlagBits::eTransferSrc) != vk::BufferUsageFlagBits::eTransferSrc)
		{
			throw std::runtime_error("Tried to resize a device local buffer that is missing the TransferSrc flag!");
		}

		VulkanBuffer new_buffer =
			VKManager::CreateBuffer(context, new_size, bufferUsage | vk::BufferUsageFlagBits::eTransferDst, memoryProperty);
		VKManager::CopyBuffer(context, this->buffer, new_buffer.buffer, this->size);

		*this = std::move(new_buffer);
	}
}

void* Backend::VulkanBuffer::GetMappedPtr()
{
	assert(data != nullptr && "Trying to retrive a pointer to unmapped memory!");
	return data;
}

void Backend::VulkanBuffer::CopyHostData(const VulkanContext& context, const void* src, uint64_t buffer_size)
{
	/* The buffer must be device local for this to work! */
	if (data)
	{
		throw std::runtime_error("This function shall not be used for Host visible buffers!");
	}

	if (buffer_size > this->size)
	{
		throw std::runtime_error("Tried to copy more data than the buffer can currently hold!");
	}

	/* Create a staging buffer for the host data */
	VulkanBuffer staging_buffer = VKManager::CreateBuffer(
		context, buffer_size, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* staging_ptr = staging_buffer.GetMappedPtr();

	/* This should always work, as the buffer was just created using HostVisible */
	assert(staging_ptr);

	std::memcpy(staging_ptr, src, buffer_size);

	VKManager::CopyBuffer(context, staging_buffer.buffer, this->buffer, buffer_size);
}
