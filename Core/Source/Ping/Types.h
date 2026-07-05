#pragma once
#include <cstdint>

namespace Ping {

	enum class ImageLayout {
		Undefined,
		ColorAttachmentOptimal,
		PresentSource,
	};

	enum class AccessMask : uint32_t {
		None = 0,
		ColorAttachmentWrite = 1 << 0,
	};

	constexpr AccessMask operator|(AccessMask lhs, AccessMask rhs) {
		return static_cast<AccessMask>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}
	constexpr bool HasFlag(AccessMask value, AccessMask flag) {
		return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
	}

	enum class PipelineStage : uint32_t {
		None = 0,
		ColorAttachmentOutput = 1 << 0,
		BottomOfPipe = 1 << 1,
	};
	constexpr PipelineStage operator|(PipelineStage lhs, PipelineStage rhs) {
		return static_cast<PipelineStage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}
	constexpr bool HasFlag(PipelineStage value, PipelineStage flag) {
		return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
	}

	struct ImageLayoutTransition {
		ImageLayout   oldLayout;
		ImageLayout   newLayout;
		AccessMask    srcAccessMask;
		AccessMask    dstAccessMask;
		PipelineStage srcStage;
		PipelineStage dstStage;
	};

	enum class QueueType
	{
		None,
		Graphics,
		Compute,
		Transfer
	};

}



