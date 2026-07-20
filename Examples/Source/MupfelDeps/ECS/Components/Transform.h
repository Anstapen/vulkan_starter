#pragma once
#include "Ping/Types.h"
#include "glm/glm.hpp"

namespace Mupfel
{

/** Per-entity position + color, uploaded directly to the GPU as a vertex via `GetVertexLayout`. */
struct Transform
{
	float pos_x = 0.0f;
	float pos_y = 0.0f;
	float pos_z = 0.0f;
	float scale_x = 1.0f;
	float scale_y = 1.0f;
	float rotation = 0.0f;
	float tilt = 0.0f;
	/** If true, rendered as an upright camera-facing billboard; if false, a flat quad in the x/y plane (the ground). */
	bool billboard = true;
	/** Texture repeat factor; values > 1 tile the texture (used by the ground). */
	float uvScale = 1.0f;
};

} // namespace Mupfel