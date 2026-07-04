#pragma once
#include <memory>
#include "Window/Window.h"
#include "SwapChain.h"
#include "Pipeline.h"

/* Forward Declaration of backend-specific classes and types */
namespace Backend {
	class VulkanContext;
}

namespace Ping {

	struct DeviceSpecification {
		
	};

	class Device {
	public:
		virtual ~Device();
		Device(const DeviceSpecification& specification, const Window &window);
		Device(const Device& other) = delete;
		Device(Device&& other);
		Device& operator=(const Device& other) = delete;
		Device& operator=(Device&& other);
	public:
		SwapChain CreateSwapChain(const Window& window);
		Pipeline CreatePipeline(const PipelineSpecification& specification, const SwapChain &swapchain);

	private:
		std::unique_ptr<Backend::VulkanContext> vulkanContextPtr;
	};

}