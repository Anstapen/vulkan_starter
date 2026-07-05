#include "VulkanTypeConversions.h"

vk::ImageLayout Backend::ToVulkan(Ping::ImageLayout layout)
{
	switch (layout) {
	case Ping::ImageLayout::Undefined:              return vk::ImageLayout::eUndefined;
	case Ping::ImageLayout::ColorAttachmentOptimal: return vk::ImageLayout::eColorAttachmentOptimal;
	case Ping::ImageLayout::PresentSource:          return vk::ImageLayout::ePresentSrcKHR;
	}
	throw std::runtime_error("Unhandled Ping::ImageLayout");
}

vk::AccessFlags2 Backend::ToVulkan(Ping::AccessMask mask)
{
	vk::AccessFlags2 result{};
	if (Ping::HasFlag(mask, Ping::AccessMask::ColorAttachmentWrite))
		result |= vk::AccessFlagBits2::eColorAttachmentWrite;
	return result;
}

vk::PipelineStageFlags2 Backend::ToVulkan(Ping::PipelineStage stage)
{
	vk::PipelineStageFlags2 result{};
	if (Ping::HasFlag(stage, Ping::PipelineStage::ColorAttachmentOutput))
		result |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	if (Ping::HasFlag(stage, Ping::PipelineStage::BottomOfPipe))
		result |= vk::PipelineStageFlagBits2::eBottomOfPipe;
	return result;
}
