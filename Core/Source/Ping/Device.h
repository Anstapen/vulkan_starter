#pragma once
#include <memory>
#include "Window/Window.h"
#include "SwapChain.h"

/* Forward Declaration of backend-specific classes and types */
namespace Backend {
	class VulkanContext;
}

namespace vk {
	struct SurfaceFormatKHR;
	enum class PresentModeKHR;
	struct Extent2D;
	struct SurfaceCapabilitiesKHR;
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

	private:
		std::unique_ptr<Backend::VulkanContext> vulkanContextPtr;
	};

}