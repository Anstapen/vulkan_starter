#pragma once

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
	GLFWwindow* GetGLFWHandle() const;
private:
	GLFWwindow* window;
};