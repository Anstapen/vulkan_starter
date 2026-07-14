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
	float _pad0 = 0.0f;
	float scale_x = 1.0f;
	float scale_y = 1.0f;
	float rotation = 0.0f;
	float _padding[1];
};

} // namespace Mupfel