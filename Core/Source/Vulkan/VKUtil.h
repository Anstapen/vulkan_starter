#pragma once
#include <vector>
#include <optional>
#include "Logger/Logger.h"
#include "vulkan/vulkan_raii.hpp"

namespace Backend
{

    class VKExtensions
    {
    public:
        VKExtensions() = default;
        VKExtensions(const VKExtensions& other) = delete;
        VKExtensions(VKExtensions&& other) = delete;
        virtual ~VKExtensions();
        const VKExtensions& operator=(const VKExtensions& other) = delete;
        VKExtensions&& operator=(VKExtensions&& other) = delete;

        void Add(const char* extension_name);
        const char* const* Data() const;
        size_t Size() const;

        static bool IsExtensionSupported(const char* extension_name);

    private:
        static std::vector<VkExtensionProperties> GetAvailableExtensions();

    private:
        std::vector<char*> extension_strings;
    };

    class VKValidationLayers
    {
    public:
        VKValidationLayers() = default;
        VKValidationLayers(const VKValidationLayers& other) = delete;
        VKValidationLayers(VKValidationLayers&& other) = delete;
        virtual ~VKValidationLayers();
        const VKValidationLayers& operator=(const VKValidationLayers& other) = delete;
        VKValidationLayers&& operator=(VKValidationLayers&& other) = delete;

        void Add(const char* validation_layer_name);
        const char* const* Data() const;
        size_t Size() const;

        static bool IsValidationLayerSupported(const char* validation_layer_name);

    private:
        static std::vector<VkLayerProperties> GetAvailableValidationLayers();

    private:
        std::vector<char*> validation_layers;
    };

    struct VKQueueFamilyProperties {
        vk::QueueFlags wanted_flags;
        uint32_t wanted_queue_instances;

        std::optional<uint32_t> GetQueueIndexFromPhysicalDevice(const vk::raii::PhysicalDevice &device) const;
		bool CheckSurfaceSupport(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface) const;
    };

}