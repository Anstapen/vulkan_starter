#pragma once
#include "VulkanCommon.h"
#include "VulkanContext.h"
#include <cstdint>
#include <string>

namespace Backend
{
class VKManager;

class VulkanImage
{
	friend class VKManager;
public:
	VulkanImage(
		vk::raii::Image&&		in_image,
		vk::raii::ImageView&&   in_image_view,
		vk::raii::DeviceMemory	in_memory,
		vk::ImageUsageFlags		in_image_usage,
		vk::MemoryPropertyFlags in_memory_property,
		uint64_t				in_size) noexcept;
	~VulkanImage();
	VulkanImage(const VulkanImage& other) = delete;
	VulkanImage(VulkanImage&& other) noexcept;
	VulkanImage& operator=(const VulkanImage& other) = delete;
	VulkanImage& operator=(VulkanImage&& other) noexcept;

private:
	/** The underlying Vulkan buffer. */
	vk::raii::Image image;
	/** The image view needed for the fragment shader. */
	vk::raii::ImageView imageView;
	/** Device memory bound to `buffer`. */
	vk::raii::DeviceMemory memory;
	/** The buffer usage type used to create that buffer. */
	vk::ImageUsageFlags imageUsage;
	/** The memory property type used to allocate the underlying memory. */
	vk::MemoryPropertyFlags memoryProperty;
	/** Size of the buffer in bytes. */
	uint64_t size;
};

} // namespace Backend
