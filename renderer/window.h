#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "type.h"

class glfwWindow
{
public:
	glfwWindow(u32 width, u32 height);
	~glfwWindow();

	glfwWindow(const glfwWindow &) = delete;
	glfwWindow operator=(const glfwWindow &) = delete;

	void init();
	bool step();

private:
	u32 width{0};
	u32 height{0};

	GLFWwindow *window{nullptr};
};
