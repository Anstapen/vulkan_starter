#pragma once
#include <memory>

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
		Device(const DeviceSpecification& specification);
		Device(const Device& other) = delete;
		Device(Device&& other);
		Device& operator=(const Device& other) = delete;
		Device& operator=(Device&& other);

	private:
		std::unique_ptr<Backend::VulkanContext> vulkanContextPtr;
	};

}