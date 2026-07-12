#include "VulkanPipeline.h"

using namespace Backend;

VulkanPipeline::VulkanPipeline(
	vk::raii::Pipeline&&						 in_pipeline,
	vk::raii::ShaderModule&&					 in_shaders,
	std::vector<vk::raii::DescriptorSetLayout>&& in_descriptor_set_layouts,
	vk::raii::PipelineLayout&&					 in_pipeline_layout) noexcept
	: pipeline(std::move(in_pipeline)), shaders(std::move(in_shaders)),
	  descriptorSetLayouts(std::move(in_descriptor_set_layouts)), pipelineLayout(std::move(in_pipeline_layout))
{
}

Backend::VulkanPipeline::VulkanPipeline() noexcept : pipeline(nullptr), shaders(nullptr), pipelineLayout(nullptr) {}

VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept
	: pipeline(std::move(other.pipeline)), shaders(std::move(other.shaders)),
	  descriptorSetLayouts(std::move(other.descriptorSetLayouts)), pipelineLayout(std::move(other.pipelineLayout))
{
}

VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept
{
	pipeline = std::move(other.pipeline);
	shaders = std::move(other.shaders);
	descriptorSetLayouts = std::move(other.descriptorSetLayouts);
	pipelineLayout = std::move(other.pipelineLayout);
	return *this;
}

VulkanPipeline::~VulkanPipeline() {}
