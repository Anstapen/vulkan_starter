#pragma once
#include <cstdint>

/* forward declarations */
struct GLFWwindow;
typedef struct GLFWwindow GLFWwindow;

class Window
{
public:
	Window();
	virtual ~Window();
	Window(const Window& other) = delete;
	Window(Window&& other);
	Window& operator=(const Window& other) = delete;
	Window& operator=(Window&& other);
public:
	bool shouldClose() const;
	void pollEvents() const;
	void waitEvents() const;
	GLFWwindow* GetGLFWHandle() const;
	void GetFramebufferSize(int32_t& width, int32_t& height) const;
public:
	volatile bool windowResized = false;
private:
	GLFWwindow* window;
};