#include "Sampler.h"
#include "Vulkan/VulkanSampler.h"

Ping::Sampler::Sampler(Backend::VulkanSampler&& in_sampler) noexcept
	: vulkanSamplerPtr(new Backend::VulkanSampler(std::move(in_sampler)))
{
}

Ping::Sampler::~Sampler() {}

Ping::Sampler::Sampler(Sampler&& other) noexcept : vulkanSamplerPtr(std::move(other.vulkanSamplerPtr)) {}

Ping::Sampler& Ping::Sampler::operator=(Sampler&& other) noexcept
{
	vulkanSamplerPtr = std::move(other.vulkanSamplerPtr);
	return *this;
}
