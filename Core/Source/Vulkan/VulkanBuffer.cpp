#include "VulkanBuffer.h"

using namespace Backend;

Backend::VulkanBuffer::VulkanBuffer(
	vk::raii::Buffer&&	   in_buffer,
	vk::raii::DeviceMemory in_memory,
	void*				   in_data,
	uint64_t			   in_size) noexcept
	: buffer(std::move(in_buffer)), memory(std::move(in_memory)), data(in_data), size(in_size)
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
	: buffer(std::move(other.buffer)), memory(std::move(other.memory)), data(other.data), size(other.size)
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
	return *this;
}

uint64_t Backend::VulkanBuffer::Size() const { return size; }

void* Backend::VulkanBuffer::GetMappedPtr()
{
	assert(data != nullptr && "Trying to retrive a pointer to unmapped memory!");
	return data;
}
