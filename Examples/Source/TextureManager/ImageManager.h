/**
 * \file   TextureManager.h
 * \brief  Load and unload images.
 *
 * \author anton
 * \date   July 2026
 */
#pragma once
#include "Error.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/** Forward declaration of the underlying image format. */
namespace Ping
{
class Image;
class Device;
} // namespace Ping

namespace Mupfel
{

class Renderer;

typedef uint32_t ImageHandle;

/**
 * This structure can be used to describe how the to-be-loaded image
 * should be interpreted. The default values define a simple image,
 * without any animation.
 */
struct AnimationSpecification
{
	/** The width of a frame. */
	uint32_t width;
	/** The height of a frame. */
	uint32_t height;
	/** The number of animations that the image contains. */
	uint32_t num_animations = 1;
	/** The number of frames per animation. */
	uint32_t frames_per_animation = 1;

	std::vector<uint32_t> animation_frames;
};

/**
 * The main image manager. It supports simple image and more advanced,
 * animated image.
 */
class ImageManager
{
	friend class Renderer;

public:
	/**
	 * Try to load the image given by \a path.
	 *
	 * \param path Path to the image.
	 * \param spec If an animated image should be loaded, this specification is used to interpret the image.
	 * \return ImageHandle or Error Code.
	 */
	[[nodiscard]] Expected<ImageHandle> Load(
		Renderer&					  renderer,
		const Ping::Device&			  device,
		const std::string			  path,
		const AnimationSpecification& spec = {});

	void Unload(const std::string path);

	void Unload(ImageHandle image);

public:
	/**
	 * The maximum number of images that can be opened concurrently.
	 */
	static constexpr uint32_t maxImageCount = 4096;

private:
	/**
	 * This vector manages the RAII-based images.
	 * ImageHandles directly translate to the vector index.
	 */
	std::vector<Ping::Image> images;
	/**
	 * This map contains a path -> ImageHandle association.
	 * Not used by the engine itself but useful for the user to
	 * be able to reference images by the path.
	 */
	std::unordered_map<std::string, ImageHandle> imageHandleMap;
};

} // namespace Mupfel
