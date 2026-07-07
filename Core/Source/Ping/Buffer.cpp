#include "Buffer.h"
#include "Vulkan/VulkanBuffer.h"

using namespace Ping;

Ping::Buffer::Buffer(Backend::VulkanBuffer&& in_buffer) noexcept
	: vulkanBufferPtr(new Backend::VulkanBuffer(std::move(in_buffer)))
{
}

Ping::Buffer::~Buffer() {}

Ping::Buffer::Buffer(Buffer&& other) noexcept : vulkanBufferPtr(std::move(other.vulkanBufferPtr)) {}

Buffer& Ping::Buffer::operator=(Buffer&& other) noexcept
{
	vulkanBufferPtr = std::move(other.vulkanBufferPtr);
	return *this;
}

uint64_t Ping::Buffer::Size() const { return vulkanBufferPtr->Size(); }

void* Ping::Buffer::GetMappedPtr() { return vulkanBufferPtr->GetMappedPtr(); }
