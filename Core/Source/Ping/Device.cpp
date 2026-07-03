#include "Vulkan/VulkanContext.h"
#include "Device.h"

using namespace Ping;

Ping::Device::~Device()
{
}

Ping::Device::Device(const DeviceSpecification& specification)
{
}

Ping::Device::Device(Device&& other) : vulkanContextPtr(std::move(other.vulkanContextPtr))
{
	other.vulkanContextPtr.reset();
}

Device& Ping::Device::operator=(Device&& other)
{
	this->vulkanContextPtr = std::move(other.vulkanContextPtr);
	return *this;
}
