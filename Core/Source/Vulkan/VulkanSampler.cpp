#include "VulkanSampler.h"

Backend::VulkanSampler::VulkanSampler(vk::raii::Sampler&& in_sampler) noexcept : sampler(std::move(in_sampler)) {}

Backend::VulkanSampler::~VulkanSampler() {}

Backend::VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept : sampler(std::move(other.sampler)) {}

Backend::VulkanSampler& Backend::VulkanSampler::operator=(VulkanSampler&& other) noexcept
{
	sampler = std::move(other.sampler);
	return *this;
}
