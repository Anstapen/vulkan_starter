#pragma once
#include "Ping/Types.h"
#include "VulkanCommon.h"
#include <stdexcept>

/**
 * @file
 * `Backend::ToVulkan` overloads bridging each `Ping` enum to its Vulkan equivalent. This is the one
 * place that translation happens; a new `Ping` enum value needs a corresponding case/flag check
 * added to its overload here (see CLAUDE.md's "Architecture" section).
 */

namespace Backend
{
/** @throws std::runtime_error if `layout` has no known `vk::ImageLayout` mapping. */
vk::ImageLayout ToVulkan(Ping::ImageLayout layout);

/** Bitwise-ORs together the `vk::AccessFlagBits2` for every flag set in `mask`. */
vk::AccessFlags2 ToVulkan(Ping::AccessMask mask);

/** Bitwise-ORs together the `vk::PipelineStageFlagBits2` for every flag set in `stage`. */
vk::PipelineStageFlags2 ToVulkan(Ping::PipelineStage stage);

/** Bitwise-ORs together the `vk::BufferUsageFlagBits` for every flag set in `usage`. */
vk::BufferUsageFlags ToVulkan(Ping::BufferUsage usage);

/** Bitwise-ORs together the `vk::CommandBufferUsageFlags` for every flag set in `usage`. */
vk::CommandBufferUsageFlags ToVulkan(Ping::CommandBufferUsage usage);

/** Bitwise-ORs together the `vk::MemoryPropertyFlagBits` for every flag set in `property`. */
vk::MemoryPropertyFlags ToVulkan(Ping::MemoryProperty property);

/** @note Aborts via `assert` if `format` has no known `vk::Format` mapping. */
vk::Format ToVulkan(Ping::VertexFormat format);
} // namespace Backend