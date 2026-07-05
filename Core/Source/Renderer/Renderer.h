#pragma once

#include <optional>
#include <cstdint>

#include "Logger/Logger.h"
#include "Ping/Device.h"

namespace Mupfel {

	class Renderer
	{
	public:
		void Init(const Ping::Device &device, const Window &window);
		void RenderNextFrame(const Ping::Device& device, const Window& window);
		void Shutdown();
	private:
		void incrementFrameIndex();
	private:
		static constexpr uint32_t frames_in_flight = 2;
		uint32_t frameIndex = 0;
		Logger::SafeLoggerPtr logger;
		std::optional<Ping::SwapChain> swapchain;
		std::optional<Ping::Pipeline> pipeline;
		std::optional<Ping::CommandBuffers> commandBuffers;
	};

}


