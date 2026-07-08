#pragma once
#include "Logger/Logger.h"
#include "VulkanCommon.h"
#include "Ping/Types.h"
#include <optional>
#include <vector>

namespace Backend
{

/**
 * Owning, null-terminated-C-string list of instance extension names to request, in the form the
 * Vulkan API wants (`const char* const*` + count). Not copyable or movable; each name is a
 * heap-allocated copy owned by this object and freed on destruction.
 */
class VKExtensions
{
public:
	VKExtensions() = default;
	VKExtensions(const VKExtensions& other) = delete;
	VKExtensions(VKExtensions&& other) = delete;
	virtual ~VKExtensions();
	const VKExtensions& operator=(const VKExtensions& other) = delete;
	VKExtensions&&		operator=(VKExtensions&& other) = delete;

	/** Copies `extension_name` into the list. A no-op if `extension_name` is empty. */
	void Add(const char* extension_name);

	/**
	 * Pointer to the underlying `const char*` array, suitable for
	 * `vk::InstanceCreateInfo::ppEnabledExtensionNames`.
	 */
	const char* const* Data() const;

	/** Number of extension names currently in the list. */
	size_t Size() const;

	/** Whether `extension_name` is present in the instance's available extensions. */
	static bool IsExtensionSupported(const char* extension_name);

private:
	/** Queries the instance-level extensions the Vulkan implementation currently supports. */
	static std::vector<VkExtensionProperties> GetAvailableExtensions();

private:
	/** Heap-allocated, null-terminated copies of each added name. */
	std::vector<char*> extension_strings;
};

/**
 * Owning, null-terminated-C-string list of validation layer names to request, in the form the
 * Vulkan API wants (`const char* const*` + count). Not copyable or movable; each name is a
 * heap-allocated copy owned by this object and freed on destruction.
 */
class VKValidationLayers
{
public:
	VKValidationLayers() = default;
	VKValidationLayers(const VKValidationLayers& other) = delete;
	VKValidationLayers(VKValidationLayers&& other) = delete;
	virtual ~VKValidationLayers();
	const VKValidationLayers& operator=(const VKValidationLayers& other) = delete;
	VKValidationLayers&&	  operator=(VKValidationLayers&& other) = delete;

	/** Copies `validation_layer_name` into the list. A no-op if `validation_layer_name` is empty. */
	void Add(const char* validation_layer_name);

	/** Pointer to the underlying `const char*` array, suitable for `vk::InstanceCreateInfo::ppEnabledLayerNames`. */
	const char* const* Data() const;

	/** Number of validation layer names currently in the list. */
	size_t Size() const;

	/** Whether `validation_layer_name` is present in the instance's available layers. */
	static bool IsValidationLayerSupported(const char* validation_layer_name);

private:
	/** Queries the instance validation layers the Vulkan implementation currently supports. */
	static std::vector<VkLayerProperties> GetAvailableValidationLayers();

private:
	/** Heap-allocated, null-terminated copies of each added name. */
	std::vector<char*> validation_layers;
};


/** A single requested queue, identified by role rather than raw Vulkan flags. */
struct VKQueueRequest
{
	Ping::QueueType type;
	/** Prefer a family that does NOT also carry other roles' flags — for a genuinely
	 *  separate hardware queue (async compute/transfer) where the GPU exposes one. */
	bool prefer_dedicated_family = false;
};

/** Where a `VKQueueRequest` ended up: which family, and which queue slot within it. */
struct VKResolvedQueue
{
	Ping::QueueType type;
	uint32_t		familyIndex;
	uint32_t		queueIndexInFamily;
};

/** Resolves `VKQueueRequest`s to concrete queue families, deduplicating so multiple
 *  requests can safely share a family in a single `vk::DeviceQueueCreateInfo`. */
class VKQueueFamilyAllocator
{
public:
	/** @throws std::runtime_error if a request's flags aren't satisfied by any queue family. */
	static std::vector<VKResolvedQueue>
	Allocate(const vk::raii::PhysicalDevice& phys_device, const std::vector<VKQueueRequest>& requests);

private:
	static vk::QueueFlags RequiredFlags(Ping::QueueType type);
};

} // namespace Backend