#include "VKUtil.h"
#include <cstring>
#include <cstdlib>
#include <cstdint>
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

const char* const* Backend::VKExtensions::Data() const
{
	return extension_strings.data();
}

size_t Backend::VKExtensions::Size() const
{
	return extension_strings.size();
}


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

const char* const* Backend::VKValidationLayers::Data() const
{
	return validation_layers.data();
}

size_t Backend::VKValidationLayers::Size() const
{
	return validation_layers.size();
}


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

std::optional<uint32_t>
Backend::VKQueueFamilyProperties::GetQueueIndexFromPhysicalDevice(
	const vk::raii::PhysicalDevice& device) const
{
	auto queue_family_properties = device.getQueueFamilyProperties();

	std::optional<uint32_t> index;
	for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
		if (((queue_family_properties[i].queueFlags &
			wanted_flags) == wanted_flags) &&
			queue_family_properties[i].queueCount >=
			wanted_queue_instances) {
			index = i;
		}
	}

	return index;
}