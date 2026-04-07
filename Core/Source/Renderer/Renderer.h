#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Mupfel {

	class Renderer
	{
	public:
		void Init();
		void Render();
		void DeInit();
	private:
		void InitWindow();
	private:
		GLFWwindow *window = nullptr;
	};

}


