#pragma once
#include "Types.h"
#include <memory>
#include <string>

namespace Backend
{
class VulkanPipeline;
}

namespace Ping
{

/** Describes the graphics pipeline to build in `Device::CreatePipeline`. */
struct PipelineSpecification
{
	/** Path to a compiled SPIR-V shader (see `Examples/Shaders/compile.bat`). */
	const std::string shaderFilePath;
	/** Vertex buffer binding and attribute layout the shader expects. */
	VertexBinding vertexLayout;
	/** Descriptor bindings the shader expects; empty if the pipeline uses no descriptor sets. */
	std::vector<DescriptorBinding> descriptorBindings;
};

class CommandBuffer;

/**
 * An RAII-owned graphics pipeline, created via `Device::CreatePipeline`.
 *
 * @note Move-only: owns the backend pipeline (and its descriptor set layout/pipeline layout) for its
 * lifetime. Must not outlive the `SwapChain` it was created against, since the pipeline is built for
 * that swapchain's render target format.
 */
class Pipeline
{
	friend class CommandBuffer;
	friend class Device;

public:
	/** Takes ownership of an existing backend pipeline. Used internally by `Device::CreatePipeline`. */
	Pipeline(Backend::VulkanPipeline&& in_pipeline);
	Pipeline(const Pipeline& other) = delete;
	/** Move-constructs from `other`, taking over its backend pipeline. */
	Pipeline(Pipeline&& other);
	Pipeline& operator=(const Pipeline& other) = delete;
	/** Move-assigns from `other`, taking over its backend pipeline. */
	Pipeline& operator=(Pipeline&& other);
	~Pipeline();

private:
	/** Owning pointer to the backend pipeline. */
	std::unique_ptr<Backend::VulkanPipeline> vulkanPipelinePtr;
};
} // namespace Ping