#include "Buffer.h"
#include "Vulkan/VulkanBuffer.h"
#include "Device.h"

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

void Ping::Buffer::Resize(const Device& device, uint64_t new_size)
{
	vulkanBufferPtr->Resize(*device.vulkanContextPtr.get(), new_size);
}

void* Ping::Buffer::GetMappedPtr() { return vulkanBufferPtr->GetMappedPtr(); }

void Ping::Buffer::CopyHostData(const Device& device, void* src, uint64_t size)
{
	vulkanBufferPtr->CopyHostData(*device.vulkanContextPtr.get(), src, size);
}
