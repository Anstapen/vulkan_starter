#pragma once
#include "VulkanCommon.h"

namespace Backend
{

/**
 * RAII-owning backend counterpart of `Ping::Pipeline`: the graphics pipeline together with the
 * shader module and pipeline layout it was built from.
 *
 * @note Move-only. Built by `VKManager::CreatePipeline`; not intended to be constructed directly
 * elsewhere.
 */
class VulkanPipeline
{
public:
	/** Takes ownership of an already-created pipeline, shader module, and pipeline layout. */
	VulkanPipeline(
		vk::raii::Pipeline&&	   in_pipeline,
		vk::raii::ShaderModule&&   in_shaders,
		vk::raii::PipelineLayout&& in_pipeline_layout) noexcept;
	/** Constructs an empty (null-handle) pipeline; not usable until move-assigned a real one. */
	VulkanPipeline() noexcept;
	VulkanPipeline(const VulkanPipeline& other) = delete;
	/** Move-constructs from `other`, taking over its pipeline, shader module, and layout. */
	VulkanPipeline(VulkanPipeline&& other) noexcept;
	VulkanPipeline& operator=(const VulkanPipeline& other) = delete;
	/** Move-assigns from `other`, taking over its pipeline, shader module, and layout. */
	VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;
	~VulkanPipeline();

public:
	/** The underlying Vulkan graphics pipeline. */
	vk::raii::Pipeline pipeline;
	/** Shader module the pipeline was built from. */
	vk::raii::ShaderModule shaders;
	/** Layout (descriptor sets/push constants) the pipeline was built with. */
	vk::raii::PipelineLayout pipelineLayout;
};

} // namespace Backend