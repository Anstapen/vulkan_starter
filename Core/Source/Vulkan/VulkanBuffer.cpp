#include "VulkanBuffer.h"
#include "VKManager.h"

using namespace Backend;

Backend::VulkanBuffer::VulkanBuffer(
	vk::raii::Buffer&&	   in_buffer,
	vk::raii::DeviceMemory in_memory,
	void*				   in_data,
	uint64_t			   in_size,
	Ping::BufferUsage	   in_buffer_usage,
	Ping::MemoryProperty   in_memory_property) noexcept
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
		/* TODO: buffer is device local */
		throw std::runtime_error("The buffer resize for device-local buffers is not yet implemented!");
	}
}

void* Backend::VulkanBuffer::GetMappedPtr()
{
	assert(data != nullptr && "Trying to retrive a pointer to unmapped memory!");
	return data;
}
