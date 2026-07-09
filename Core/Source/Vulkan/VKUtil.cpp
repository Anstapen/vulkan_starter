#include "VKUtil.h"
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

Backend::VKExtensions::~VKExtensions()
{
	for (uint32_t i = 0; i < extension_strings.size(); i++)
	{
		free(extension_strings[i]);
	}

	extension_strings.clear();
}

void Backend::VKExtensions::Add(const char* extension_name)
{
	if (strlen(extension_name) == 0)
	{
		return;
	}

	size_t bytes = strlen(extension_name) + 1;

	char* new_string = (char*)malloc(bytes);
	if (new_string == nullptr)
	{
		throw std::runtime_error("Failed to allocate memory for validation layer name.");
	}
	strncpy(new_string, extension_name, strlen(extension_name));
	new_string[strlen(extension_name)] = '\0';

	extension_strings.push_back(new_string);
}

const char* const* Backend::VKExtensions::Data() const { return extension_strings.data(); }

size_t Backend::VKExtensions::Size() const { return extension_strings.size(); }

bool Backend::VKExtensions::IsExtensionSupported(const char* extension_name)
{
	std::vector<VkExtensionProperties> extensions = GetAvailableExtensions();

	for (auto& e : extensions)
	{
		if (strncmp(e.extensionName, extension_name, VK_MAX_EXTENSION_NAME_SIZE) == 0)
		{
			return true;
		}
	}

	return false;
}

std::vector<VkExtensionProperties> Backend::VKExtensions::GetAvailableExtensions()
{
	uint32_t extension_count = 0;
	VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	if (result != VK_SUCCESS)
	{
		return {};
	}

	if (extension_count == 0)
	{
		return {};
	}

	std::vector<VkExtensionProperties> properties(extension_count);

	result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, properties.data());

	if (result != VK_SUCCESS)
	{
		return {};
	}

	return properties;
}

Backend::VKValidationLayers::~VKValidationLayers()
{
	for (uint32_t i = 0; i < validation_layers.size(); i++)
	{
		free(validation_layers[i]);
	}

	validation_layers.clear();
}

void Backend::VKValidationLayers::Add(const char* validation_layer_name)
{
	if (strlen(validation_layer_name) == 0)
	{
		return;
	}

	size_t bytes = strlen(validation_layer_name) + 1;

	char* new_string = (char*)malloc(bytes);
	if (new_string == nullptr)
	{
		throw std::runtime_error("Failed to allocate memory for validation layer name.");
	}
	strncpy(new_string, validation_layer_name, strlen(validation_layer_name));
	new_string[strlen(validation_layer_name)] = '\0';

	validation_layers.push_back(new_string);
}

const char* const* Backend::VKValidationLayers::Data() const { return validation_layers.data(); }

size_t Backend::VKValidationLayers::Size() const { return validation_layers.size(); }

bool Backend::VKValidationLayers::IsValidationLayerSupported(const char* validation_layer_name)
{
	std::vector<VkLayerProperties> validation_layers = GetAvailableValidationLayers();

	for (auto& e : validation_layers)
	{
		if (strncmp(e.layerName, validation_layer_name, VK_MAX_EXTENSION_NAME_SIZE) == 0)
		{
			return true;
		}
	}

	return false;
}

std::vector<VkLayerProperties> Backend::VKValidationLayers::GetAvailableValidationLayers()
{
	uint32_t layer_count = 0;
	VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	if (result != VK_SUCCESS)
	{
		return {};
	}

	if (layer_count == 0)
	{
		return {};
	}

	std::vector<VkLayerProperties> available_layers(layer_count);
	result = vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	if (result != VK_SUCCESS)
	{
		return {};
	}

	return available_layers;
}

vk::QueueFlags Backend::VKQueueFamilyAllocator::RequiredFlags(Ping::QueueType type)
{
	switch (type)
	{
	case Ping::QueueType::Graphics:
		return vk::QueueFlagBits::eGraphics;
	case Ping::QueueType::Compute:
		return vk::QueueFlagBits::eCompute;
	case Ping::QueueType::Transfer:
		return vk::QueueFlagBits::eTransfer;
	default:
		throw std::runtime_error("No queue flags defined for this QueueType!");
	}
}

std::vector<Backend::VKResolvedQueue> Backend::VKQueueFamilyAllocator::Allocate(
	const vk::raii::PhysicalDevice&	   phys_device,
	const std::vector<VKQueueRequest>& requests)
{
	std::vector<vk::QueueFamilyProperties> families = phys_device.getQueueFamilyProperties();
	std::vector<uint32_t>				   claimed(families.size(), 0); // queue slots already handed out, per family

	/* Fewer capability bits == more "dedicated" a family is: a transfer-only family scores 1,
	 * a combined graphics+compute+transfer family scores 3. Picking the lowest-scoring match
	 * is how a dedicated request lands on a genuinely separate hardware queue when one exists,
	 * without hardcoding "must lack eGraphics" (which wouldn't generalize to future roles). */
	auto specializationScore = [](vk::QueueFlags flags)
	{ return std::popcount(static_cast<uint32_t>(static_cast<VkQueueFlags>(flags))); };

	auto findBestFamily = [&](vk::QueueFlags required, bool requireSpareCapacity) -> std::optional<uint32_t>
	{
		std::optional<uint32_t> best;
		for (uint32_t i = 0; i < families.size(); i++)
		{
			if ((families[i].queueFlags & required) != required)
				continue;
			if (requireSpareCapacity && claimed[i] >= families[i].queueCount)
				continue;
			if (!best.has_value() ||
				specializationScore(families[i].queueFlags) < specializationScore(families[*best].queueFlags))
			{
				best = i;
			}
		}
		return best;
	};

	auto claimSlot = [&](uint32_t family) -> uint32_t
	{
		uint32_t queueIndex = std::min(claimed[family], families[family].queueCount - 1);
		claimed[family] = queueIndex + 1;
		return queueIndex;
	};

	auto resolveOne = [&](const VKQueueRequest& request) -> VKResolvedQueue
	{
		vk::QueueFlags			required = RequiredFlags(request.type);
		std::optional<uint32_t> family = findBestFamily(required, /*requireSpareCapacity=*/true);
		if (!family.has_value())
			family = findBestFamily(required, /*requireSpareCapacity=*/false); // share, nothing free left
		if (!family.has_value())
			throw std::runtime_error("No queue family satisfies a requested queue's flags!");
		return {request.type, *family, claimSlot(*family)};
	};

	std::vector<VKResolvedQueue> resolved(requests.size());

	/* Two passes: dedicated requests get first pick of scarce specialized families, before a
	 * "whatever's available" request can greedily consume the only transfer-only family etc. */
	for (uint32_t i = 0; i < requests.size(); i++)
		if (requests[i].prefer_dedicated_family)
			resolved[i] = resolveOne(requests[i]);

	for (uint32_t i = 0; i < requests.size(); i++)
		if (!requests[i].prefer_dedicated_family)
			resolved[i] = resolveOne(requests[i]);

	return resolved;
}