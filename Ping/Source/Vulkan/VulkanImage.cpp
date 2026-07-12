#include "VulkanImage.h"
#include "VKManager.h"
#include "VulkanTypeConversions.h"

Backend::VulkanImage::VulkanImage(
	vk::raii::Image&&		in_image,
	vk::raii::ImageView&&   in_image_view,
	vk::raii::DeviceMemory	in_memory,
	vk::ImageUsageFlags		in_image_usage,
	vk::MemoryPropertyFlags in_memory_property,
	uint64_t				in_size) noexcept
	: image(std::move(in_image)), imageView(std::move(in_image_view)), memory(std::move(in_memory)), imageUsage(in_image_usage),
	  memoryProperty(in_memory_property), size(in_size)
{
}

Backend::VulkanImage::~VulkanImage() {}

Backend::VulkanImage::VulkanImage(VulkanImage&& other) noexcept
	: image(std::move(other.image)), imageView(std::move(other.imageView)), memory(std::move(other.memory)),
	  imageUsage(other.imageUsage),
	  memoryProperty(other.memoryProperty), size(other.size)
{
}

Backend::VulkanImage& Backend::VulkanImage::operator=(VulkanImage&& other) noexcept
{
	image = std::move(other.image);
	imageView = std::move(other.imageView);
	memory = std::move(other.memory);
	imageUsage = other.imageUsage;
	memoryProperty = other.memoryProperty;
	size = other.size;
	other.size = 0;

	return *this;
}