#pragma once
#include <memory>
#include "Window/Window.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "CommandBuffer.h"
#include "Types.h"

/* Forward Declaration of backend-specific classes and types */
namespace Backend {
	class VulkanContext;
}

namespace Ping {

	struct DeviceSpecification {
		
	};

	class Device {
		friend class CommandBuffer;
		friend class SwapChain;
	public:
		virtual ~Device();
		Device(const DeviceSpecification& specification, const Window &window);
		Device(const Device& other) = delete;
		Device(Device&& other);
		Device& operator=(const Device& other) = delete;
		Device& operator=(Device&& other);
	public:
		SwapChain CreateSwapChain(const Window& window) const;
		Pipeline CreatePipeline(const PipelineSpecification& specification, const SwapChain &swapchain) const;
		CommandBuffers CreateCommandBuffers(QueueType buffer_type, uint32_t num_buffers) const;
		void WaitForCommands() const;

	private:
		std::unique_ptr<Backend::VulkanContext> vulkanContextPtr;
	};

}