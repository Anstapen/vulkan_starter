#pragma once
#include "Ping/Types.h"
#include "glm.hpp"

namespace Mupfel
{

/** Per-entity position + color, uploaded directly to the GPU as a vertex via `GetVertexLayout`. */
struct Transform
{
	/** 2D position, bound to vertex shader location 0. */
	glm::vec2 pos;
	/** RGB color, bound to vertex shader location 1. */
	glm::vec3 color;

	/** The `Ping::VertexBinding` matching `Transform`'s memory layout, for `PipelineSpecification::vertexLayout`. */
	static Ping::VertexBinding GetVertexLayout()
	{
		return {
			.binding = 0,
			.stride = sizeof(Transform),
			.inputRate = Ping::VertexInputRate::Vertex,
			.attributes = {
				{0, Ping::VertexFormat::Float32x2, offsetof(Transform, pos)},
				{1, Ping::VertexFormat::Float32x3, offsetof(Transform, color)}}};
	}
};

} // namespace Mupfel
