#pragma once

namespace Mupfel
{
/** Placeholder color component; currently just a flat RGBA tint, not an actual texture reference. */
struct Texture
{
	/** Red channel, in [0, 1]. */
	float red = 0.0f;
	/** Green channel, in [0, 1]. */
	float green = 0.0f;
	/** Blue channel, in [0, 1]. */
	float blue = 0.0f;
	/** Alpha channel, in [0, 1]. */
	float alpha = 0.0f;
};
} // namespace Mupfel