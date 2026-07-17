#pragma once
#include "Types.h"
#include <memory>
#include <string>

namespace Backend
{
class VulkanImage;
}

namespace Ping
{
class Device;
class CommandBuffer;

class Image
{
	friend class Device;
	friend class CommandBuffer;

public:
	Image(Backend::VulkanImage&& in_image) noexcept;
	~Image();
	Image(const Image& other) = delete;
	Image(Image&& other) noexcept;
	Image& operator=(const Image& other) = delete;
	Image& operator=(Image&& other) noexcept;

private:
	/** Owning pointer to the backend buffer and its memory. */
	std::unique_ptr<Backend::VulkanImage> vulkanImagePtr;
};

} // namespace Ping