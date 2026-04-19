#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

namespace Mupfel {

	class Renderer
	{
	public:
		void Init();
		void Render();
		void DeInit();
	private:
		void InitWindow();
		void InitVulkan();
		void createInstance();
	private:
		GLFWwindow *window = nullptr;

		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;

		const std::vector<char const*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
	};

}


