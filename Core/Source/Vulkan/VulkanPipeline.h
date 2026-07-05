#pragma once
#include "VulkanCommon.h"

namespace Backend {

	class VulkanPipeline
	{
	public:
		VulkanPipeline(vk::raii::Pipeline&& in_pipeline, vk::raii::ShaderModule &&in_shaders, vk::raii::PipelineLayout &&in_pipeline_layout) noexcept;
		VulkanPipeline() noexcept;
		VulkanPipeline(const VulkanPipeline& other) = delete;
		VulkanPipeline(VulkanPipeline&& other) noexcept;
		VulkanPipeline& operator=(const VulkanPipeline& other) = delete;
		VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;
		~VulkanPipeline();
	public:
		vk::raii::Pipeline pipeline;
		vk::raii::ShaderModule shaders;
		vk::raii::PipelineLayout pipelineLayout;
	};

}