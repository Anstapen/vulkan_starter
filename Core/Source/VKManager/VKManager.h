#pragma once
#include "Logger/Logger.h"
#include "VKUtil.h"
#include "vulkan/vulkan.h"
#include <array>
#include <vector>

namespace Mupfel {

    class VKManager {
    public:
        static void Init();
        static VkInstance CreateInstance();
        static VkInstance
            CreateInstance(const std::vector<const char*>& wanted_extensions,
                const std::vector<const char*>& wanted_validation_layers);
        static void DestroyInstance(VkInstance instance);
        static void SetupDebugCallback(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT user_callback);
        static VkDevice SelectBestDevice(VkInstance instance,
            const std::vector<VKQueueFamilyProperties>& wanted_queues);
        static void CleanUpDevice(VkDevice device);

    private:
        static void AddRequiredExtensions(VKExtensions& extensions);
        static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger);
        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT in_debug_messenger, const VkAllocationCallbacks* pAllocator);
        static bool IsDeviceSuitable(VkPhysicalDevice device);
        static VkDevice CreateLogicalDevice(VkPhysicalDevice phys_device,
            const std::vector<VKQueueFamilyProperties>& wanted_queues,
            const std::vector<const char*>& wanted_extensions);

    private:
        static Logger::SafeLoggerPtr logger;
        static std::array<char*, 32> extension_array;
        static VkDebugUtilsMessengerEXT debugMessenger;
    };

} // namespace vulk
