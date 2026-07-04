#include "Pipeline.h"
#include "Vulkan/VulkanPipeline.h"

using namespace Ping;

Pipeline::Pipeline(Backend::VulkanPipeline&& in_pipeline) : vulkanPipelinePtr(std::make_unique<Backend::VulkanPipeline>(std::move(in_pipeline)))
{
}

Ping::Pipeline::Pipeline(Pipeline&& other) : vulkanPipelinePtr(std::move(other.vulkanPipelinePtr))
{
}

Pipeline& Pipeline::operator=(Pipeline&& other)
{
	vulkanPipelinePtr = std::move(other.vulkanPipelinePtr);
	return *this;
}

Pipeline::~Pipeline()
{
}
