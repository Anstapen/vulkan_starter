#include "Image.h"
#include "Device.h"
#include "Vulkan/VulkanImage.h"
#include "Vulkan/VulkanTypeConversions.h"

Ping::Image::Image(Backend::VulkanImage&& in_image) noexcept : vulkanImagePtr(new Backend::VulkanImage(std::move(in_image))) {}

Ping::Image::~Image() {}

Ping::Image::Image(Image&& other) noexcept : vulkanImagePtr(std::move(other.vulkanImagePtr)) {}

Ping::Image& Ping::Image::operator=(Image&& other) noexcept
{
	vulkanImagePtr = std::move(other.vulkanImagePtr);
	return *this;
}
