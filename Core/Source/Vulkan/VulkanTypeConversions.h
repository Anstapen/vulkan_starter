#pragma once
#include "Ping/Types.h"
#include "VulkanCommon.h"
#include <stdexcept>

namespace Backend {
	vk::ImageLayout ToVulkan(Ping::ImageLayout layout);

	vk::AccessFlags2 ToVulkan(Ping::AccessMask mask);

	vk::PipelineStageFlags2 ToVulkan(Ping::PipelineStage stage);
}