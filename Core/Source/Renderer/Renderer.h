#pragma once

#include <optional>

#include "Logger/Logger.h"
#include "Ping/Device.h"

namespace Mupfel {

	class Renderer
	{
	public:
		void Init(const Ping::Device &device, const Window &window);
		void RenderNextFrame(const Ping::Device& device);
		void Shutdown();
	private:
		Logger::SafeLoggerPtr logger;
		std::optional<Ping::SwapChain> swapchain;
		std::optional<Ping::Pipeline> pipeline;
		std::optional<Ping::CommandBuffers> commandBuffers;
	};

}


