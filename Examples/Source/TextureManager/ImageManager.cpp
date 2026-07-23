#include "ImageManager.h"
#include "Ping/Device.h"
#include "Ping/Image.h"
#include "Renderer/Renderer.h"
#include <optional>

using namespace Mupfel;

Expected<ImageHandle> Mupfel::ImageManager::Load(
	Renderer&					  renderer,
	const Ping::Device&			  device,
	const std::string			  path,
	const AnimationSpecification& spec)
{
	/* without spec for now */
	(void)spec;

	if (imageHandleMap.contains(path))
	{
		/* Image is already loaded. */
		return imageHandleMap[path];
	}

	std::optional<Ping::Image> image = device.CreateImage(path, Ping::ImageUsage::Sampled);

	if (!image.has_value())
	{
		return std::unexpected<Error>(Error::FILE_NOT_FOUND);
	}

	ImageHandle handle = static_cast<uint32_t>(images.size());

	images.emplace_back(std::move(image.value()));

	imageHandleMap[path] = handle;

	/* Images have been changed, notify the Renderer to update the descriptor sets. */
	renderer.SetImageBuffer(device, images);

	return handle;
}

void Mupfel::ImageManager::Unload(const std::string path)
{
	if (!imageHandleMap.contains(path))
	{
		return;
	}

	Unload(imageHandleMap[path]);
}

void Mupfel::ImageManager::Unload(ImageHandle image) {}
