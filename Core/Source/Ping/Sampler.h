#pragma once
#include <memory>

namespace Backend
{
class VulkanSampler;
}

namespace Ping
{
class Device;

/**
 * Describes how a shader samples an `Image` (filtering, addressing, mipmapping), created via
 * `Device::CreateSampler`.
 *
 * @note Move-only. A single `Sampler` is typically shared across many `Image`s with matching
 * sampling parameters, rather than created per-image.
 */
class Sampler
{
	friend class Device;

public:
	/** Takes ownership of an existing backend sampler. Used internally by `Device::CreateSampler`. */
	Sampler(Backend::VulkanSampler&& in_sampler) noexcept;
	~Sampler();
	Sampler(const Sampler& other) = delete;
	/** Move-constructs from `other`, taking over its backend sampler. */
	Sampler(Sampler&& other) noexcept;
	Sampler& operator=(const Sampler& other) = delete;
	/** Move-assigns from `other`, taking over its backend sampler. */
	Sampler& operator=(Sampler&& other) noexcept;

private:
	/** Owning pointer to the backend sampler. */
	std::unique_ptr<Backend::VulkanSampler> vulkanSamplerPtr;
};

} // namespace Ping
