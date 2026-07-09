#pragma once
#include <cstdint>
#include <vector>

namespace Ping
{

/**
 * Backend-agnostic mirror of `vk::ImageLayout`, restricted to the layouts this RHI currently uses
 * for swapchain image transitions.
 *
 * @see Backend::ToVulkan(ImageLayout) for the exact `vk::ImageLayout` each value maps to.
 */
enum class ImageLayout
{
	/** Contents are undefined; only valid as the "old" layout of a transition. */
	Undefined,
	/** Optimal for use as a color attachment during rendering. */
	ColorAttachmentOptimal,
	/** Required layout before an image is handed to `SwapChain::Present`. */
	PresentSource,
};

/**
 * Bitmask mirror of `vk::AccessFlags2`, restricted to the access types this RHI currently
 * synchronizes against in `ImageLayoutTransition`. Combine values with `operator|`.
 */
enum class AccessMask : uint32_t
{
	None = 0,
	/** Write access to a color attachment. */
	ColorAttachmentWrite = 1 << 0,
};

/** Combines two AccessMask flags. See the `Ping/Types.h` bitmask-enum pattern documented in CLAUDE.md. */
constexpr AccessMask operator|(AccessMask lhs, AccessMask rhs)
{
	return static_cast<AccessMask>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/** Returns whether every bit set in `flag` is also set in `value`. */
constexpr bool HasFlag(AccessMask value, AccessMask flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * Bitmask mirror of `vk::PipelineStageFlags2`, restricted to the stages this RHI currently
 * synchronizes against in `ImageLayoutTransition`. Combine values with `operator|`.
 */
enum class PipelineStage : uint32_t
{
	None = 0,
	/** The stage that writes color attachment output. */
	ColorAttachmentOutput = 1 << 0,
	/** The final stage of the pipeline; used as a synchronization sentinel. */
	BottomOfPipe = 1 << 1,
};

/** Combines two PipelineStage flags. See the `Ping/Types.h` bitmask-enum pattern documented in CLAUDE.md. */
constexpr PipelineStage operator|(PipelineStage lhs, PipelineStage rhs)
{
	return static_cast<PipelineStage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/** Returns whether every bit set in `flag` is also set in `value`. */
constexpr bool HasFlag(PipelineStage value, PipelineStage flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * Describes an image layout transition (as passed to `CommandBuffer::transitionImageLayout`):
 * the layout change plus the access/stage masks needed to synchronize around it.
 */
struct ImageLayoutTransition
{
	/** Layout the image is transitioning from. */
	ImageLayout oldLayout;
	/** Layout the image is transitioning to. */
	ImageLayout newLayout;
	/** Access types that must complete before the transition. */
	AccessMask srcAccessMask;
	/** Access types that must wait for the transition to complete. */
	AccessMask dstAccessMask;
	/** Pipeline stage(s) that must complete before the transition. */
	PipelineStage srcStage;
	/** Pipeline stage(s) that must wait for the transition to complete. */
	PipelineStage dstStage;
};

/** Identifies which queue family a set of command buffers should be created against. */
enum class QueueType
{
	/** No queue selected; not valid for `Device::CreateCommandBuffers`. */
	None,
	/** Graphics queue, for rendering commands. */
	Graphics,
	/** Compute queue, for compute dispatches. */
	Compute,
	/** Transfer queue, for buffer/image copies. */
	Transfer
};

/**
 * Bitmask mirror of `vk::BufferUsageFlags`, restricted to the usages `Device::CreateBuffer`
 * currently supports. Combine values with `operator|`.
 */
enum class BufferUsage : uint32_t
{
	None = 0,
	/** Buffer can be bound as a vertex buffer. */
	VertexBuffer = 1 << 0,
	/** Buffer can be the source of a transfer/copy operation. */
	TransferSrc = 1 << 1,
	/** Buffer can be the destination of a transfer/copy operation. */
	TransferDst = 1 << 2,
	/** Buffer can be bound as an index buffer. */
	IndexBuffer = 1 << 3,
	/** Buffer can be bound as a uniform buffer object (UBO) */
	UniformBuffer = 1 << 4
};

/** Combines two BufferUsage flags. See the `Ping/Types.h`. */
constexpr BufferUsage operator|(BufferUsage lhs, BufferUsage rhs)
{
	return static_cast<BufferUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/** Returns whether every bit set in `flag` is also set in `value`. */
constexpr bool HasFlag(BufferUsage value, BufferUsage flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * Bitmask mirror of `vk::CommandBufferUsageFlags`.
 */
enum class CommandBufferUsage : uint32_t
{
	None = 0,
	/** CommandBuffer will be submitted once */
	OneTimeSubmit = 1 << 0,
	/** Buffer can be the source of a transfer/copy operation. */
	RenderPassContinue = 1 << 1,
	/** Buffer can be the destination of a transfer/copy operation. */
	SimultaneousUse = 1 << 2
};

/** Combines two CommandBufferUsage flags. See the `Ping/Types.h`. */
constexpr CommandBufferUsage operator|(CommandBufferUsage lhs, CommandBufferUsage rhs)
{
	return static_cast<CommandBufferUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/** Returns whether every bit set in `flag` is also set in `value`. */
constexpr bool HasFlag(CommandBufferUsage value, CommandBufferUsage flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * Bitmask mirror of `vk::MemoryPropertyFlags`, restricted to the properties `Device::CreateBuffer`
 * currently supports. Combine values with `operator|`.
 */
enum class MemoryProperty : uint32_t
{
	None = 0,
	/** Memory can be mapped with `Buffer::GetMappedPtr`. */
	HostVisible = 1 << 0,
	/** Host writes are visible to the device without an explicit flush. */
	HostCoherent = 1 << 1,
	/** Memory is on device-local (fastest-for-the-GPU) heap. */
	DeviceLocal = 1 << 2
};

/** Combines two MemoryProperty flags. See the `Ping/Types.h` bitmask-enum pattern documented in CLAUDE.md. */
constexpr MemoryProperty operator|(MemoryProperty lhs, MemoryProperty rhs)
{
	return static_cast<MemoryProperty>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/** Returns whether every bit set in `flag` is also set in `value`. */
constexpr bool HasFlag(MemoryProperty value, MemoryProperty flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/** Per-attribute vertex data format, mirroring the subset of `vk::Format` this RHI exposes. */
enum class VertexFormat
{
	/** Two 32-bit floats (maps to `vk::Format::eR32G32Sfloat`). */
	Float32x2,
	/** Three 32-bit floats (maps to `vk::Format::eR32G32B32Sfloat`). */
	Float32x3,
	/** Four 32-bit floats (maps to `vk::Format::eR32G32B32A32Sfloat`). */
	Float32x4
};

/** Whether a vertex binding advances per-vertex or per-instance. */
enum class VertexInputRate
{
	/** Binding advances for every vertex. */
	Vertex,
	/** Binding advances for every instance. */
	Instance
};

/** Describes a single vertex shader input within a `VertexBinding`. */
struct VertexAttribute
{
	/** Shader `location` this attribute is bound to. */
	uint32_t location;
	/** Data format of the attribute. */
	VertexFormat format;
	/** Byte offset of the attribute within one vertex of its binding. */
	uint32_t offset;
};

/** Describes one vertex buffer binding slot and its attribute layout, used by `PipelineSpecification`. */
struct VertexBinding
{
	/** Binding slot index passed to `CommandBuffer::BindVertexBuffer`. */
	uint32_t binding;
	/** Byte size of one vertex in this binding. */
	uint32_t stride;
	/** Whether the binding advances per-vertex or per-instance. */
	VertexInputRate inputRate;
	/** Shader-visible attributes sourced from this binding. */
	std::vector<VertexAttribute> attributes;
};

/**
 * Bitmask mirror of `vk::ShaderStageFlags`, restricted to the stages this RHI's pipelines support.
 * Combine values with `operator|`.
 */
enum class ShaderStage : uint32_t
{
	None = 0,
	/** The vertex shader stage. */
	Vertex = 1 << 0,
	/** The fragment shader stage. */
	Fragment = 1 << 1,
};

/** Combines two ShaderStage flags. See the `Ping/Types.h` bitmask-enum pattern documented in CLAUDE.md. */
constexpr ShaderStage operator|(ShaderStage lhs, ShaderStage rhs)
{
	return static_cast<ShaderStage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/** Returns whether every bit set in `flag` is also set in `value`. */
constexpr bool HasFlag(ShaderStage value, ShaderStage flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * Type of resource a `DescriptorBinding` exposes to shaders, mirroring the subset of
 * `vk::DescriptorType` this RHI currently supports.
 */
enum class DescriptorType
{
	/** A uniform buffer object, bound via a `Buffer` created with `BufferUsage::UniformBuffer`. */
	UniformBuffer,
};

/** Describes one shader-visible binding slot within a pipeline's descriptor set layout. */
struct DescriptorBinding
{
	/** Binding index, matching `layout(binding = N)` in the shader. */
	uint32_t binding;
	/** Resource type exposed at this binding. */
	DescriptorType type;
	/** Shader stage(s) that read this binding. */
	ShaderStage stageFlags;
};

} // namespace Ping
