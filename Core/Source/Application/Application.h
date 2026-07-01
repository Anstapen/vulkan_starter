#pragma once
#include <string>
#include "Renderer/Renderer.h"
#include "Logger/Logger.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Mupfel
{
	class Application
	{
    public:
        Application(const std::string& in_name);

    public:
        void Run();

    private:
        void Init();
        void CreateVulkanInstance();
        void CreateGLFWWindow();
        void MainLoop();
        void CleanUp();
        static VKAPI_ATTR VkBool32 VKAPI_CALL
            debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData);

    private:
        const std::string name;
        Renderer renderer;
        Logger::SafeLoggerPtr logger;
        static Logger::SafeLoggerPtr validation_layer_logger;
        GLFWwindow* window;
        VkInstance instance;
        VkDevice device;
	};

}