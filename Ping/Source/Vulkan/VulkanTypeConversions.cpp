#include "VulkanTypeConversions.h"

vk::ImageLayout Backend::ToVulkan(Ping::ImageLayout layout)
{
	switch (layout)
	{
	case Ping::ImageLayout::Undefined:
		return vk::ImageLayout::eUndefined;
	case Ping::ImageLayout::ColorAttachmentOptimal:
		return vk::ImageLayout::eColorAttachmentOptimal;
	case Ping::ImageLayout::PresentSource:
		return vk::ImageLayout::ePresentSrcKHR;
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

vk::BufferUsageFlags Backend::ToVulkan(Ping::BufferUsage usage)
{
	vk::BufferUsageFlags result{};
	if (Ping::HasFlag(usage, Ping::BufferUsage::TransferDst))
		result |= vk::BufferUsageFlagBits::eTransferDst;
	if (Ping::HasFlag(usage, Ping::BufferUsage::TransferSrc))
		result |= vk::BufferUsageFlagBits::eTransferSrc;
	if (Ping::HasFlag(usage, Ping::BufferUsage::VertexBuffer))
		result |= vk::BufferUsageFlagBits::eVertexBuffer;
	if (Ping::HasFlag(usage, Ping::BufferUsage::IndexBuffer))
		result |= vk::BufferUsageFlagBits::eIndexBuffer;
	if (Ping::HasFlag(usage, Ping::BufferUsage::UniformBuffer))
		result |= vk::BufferUsageFlagBits::eUniformBuffer;
	if (Ping::HasFlag(usage, Ping::BufferUsage::StorageBuffer))
		result |= vk::BufferUsageFlagBits::eStorageBuffer;
	return result;
}

vk::CommandBufferUsageFlags Backend::ToVulkan(Ping::CommandBufferUsage usage)
{
	vk::CommandBufferUsageFlags result{};
	if (Ping::HasFlag(usage, Ping::CommandBufferUsage::OneTimeSubmit))
		result |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	if (Ping::HasFlag(usage, Ping::CommandBufferUsage::RenderPassContinue))
		result |= vk::CommandBufferUsageFlagBits::eRenderPassContinue;
	if (Ping::HasFlag(usage, Ping::CommandBufferUsage::SimultaneousUse))
		result |= vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	return result;
}

vk::MemoryPropertyFlags Backend::ToVulkan(Ping::MemoryProperty property)
{
	vk::MemoryPropertyFlags result{};
	if (Ping::HasFlag(property, Ping::MemoryProperty::DeviceLocal))
		result |= vk::MemoryPropertyFlagBits::eDeviceLocal;
	if (Ping::HasFlag(property, Ping::MemoryProperty::HostVisible))
		result |= vk::MemoryPropertyFlagBits::eHostVisible;
	if (Ping::HasFlag(property, Ping::MemoryProperty::HostCoherent))
		result |= vk::MemoryPropertyFlagBits::eHostCoherent;
	return result;
}

vk::Format Backend::ToVulkan(Ping::VertexFormat format)
{
	switch (format)
	{
	case Ping::VertexFormat::Float32x2:
		return vk::Format::eR32G32Sfloat;
	case Ping::VertexFormat::Float32x3:
		return vk::Format::eR32G32B32Sfloat;
	case Ping::VertexFormat::Float32x4:
		return vk::Format::eR32G32B32A32Sfloat;
	}
	throw std::runtime_error("Unhandled Ping::VertexFormat");
}

vk::ShaderStageFlags Backend::ToVulkan(Ping::ShaderStage stage)
{
	vk::ShaderStageFlags result{};
	if (Ping::HasFlag(stage, Ping::ShaderStage::Vertex))
		result |= vk::ShaderStageFlagBits::eVertex;
	if (Ping::HasFlag(stage, Ping::ShaderStage::Fragment))
		result |= vk::ShaderStageFlagBits::eFragment;
	return result;
}

vk::DescriptorType Backend::ToVulkan(Ping::DescriptorType type)
{
	switch (type)
	{
	case Ping::DescriptorType::UniformBuffer:
		return vk::DescriptorType::eUniformBuffer;
	case Ping::DescriptorType::CombinedImageSampler:
		return vk::DescriptorType::eCombinedImageSampler;
	case Ping::DescriptorType::StorageBuffer:
		return vk::DescriptorType::eStorageBuffer;
	}
	throw std::runtime_error("Unhandled Ping::DescriptorType");
}

vk::ImageUsageFlags Backend::ToVulkan(Ping::ImageUsage usage)
{
	vk::ImageUsageFlags result{};
	if (Ping::HasFlag(usage, Ping::ImageUsage::Sampled))
		result |= vk::ImageUsageFlagBits::eSampled;
	return result;
}

vk::Filter Backend::ToVulkan(Ping::SamplerFilterMode filter)
{
	switch (filter)
	{
	case Ping::SamplerFilterMode::Linear:
		return vk::Filter::eLinear;
	case Ping::SamplerFilterMode::Nearest:
		return vk::Filter::eNearest;
	}
	throw std::runtime_error("Unhandled Ping::SamplerFilterMode");
}

vk::SamplerMipmapMode Backend::ToVulkan(Ping::SamplerMipMapMode mipmap_mode)
{
	switch (mipmap_mode)
	{
	case Ping::SamplerMipMapMode::Linear:
		return vk::SamplerMipmapMode::eLinear;
	case Ping::SamplerMipMapMode::Nearest:
		return vk::SamplerMipmapMode::eNearest;
	}
	throw std::runtime_error("Unhandled Ping::SamplerMipMapMode");
}

vk::SamplerAddressMode Backend::ToVulkan(Ping::SamplerAddressMode address_mode)
{
	switch (address_mode)
	{
	case Ping::SamplerAddressMode::Repeat:
		return vk::SamplerAddressMode::eRepeat;
	case Ping::SamplerAddressMode::MirroredRepeat:
		return vk::SamplerAddressMode::eMirroredRepeat;
	case Ping::SamplerAddressMode::ClampToEdge:
		return vk::SamplerAddressMode::eClampToEdge;
	case Ping::SamplerAddressMode::MirrorClampToEdge:
		return vk::SamplerAddressMode::eMirrorClampToEdge;
	case Ping::SamplerAddressMode::ClampToBorder:
		return vk::SamplerAddressMode::eClampToBorder;
	}
	throw std::runtime_error("Unhandled Ping::SamplerAddressMode");
}
