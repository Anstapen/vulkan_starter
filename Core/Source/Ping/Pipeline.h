#pragma once
#include <memory>
#include <string>

namespace Backend {
	class VulkanPipeline;
}

namespace Ping {

	struct PipelineSpecification {
		const std::string shaderFilePath;
	};

	class Pipeline
	{
	public:
		Pipeline(Backend::VulkanPipeline &&in_pipeline);
		Pipeline(const Pipeline& other) = delete;
		Pipeline(Pipeline&& other);
		Pipeline& operator=(const Pipeline& other) = delete;
		Pipeline& operator=(Pipeline&& other);
		~Pipeline();
	private:
		std::unique_ptr<Backend::VulkanPipeline> vulkanPipelinePtr;
	};
}