#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <utils/type.h>

class glfwWindow
{
public:
	glfwWindow();
	~glfwWindow();

	glfwWindow(const glfwWindow &) = delete;
	glfwWindow operator=(const glfwWindow &) = delete;

	void init(u32 width, u32 height);

	bool step();

	VkResult createVulkanSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
	u32 width{ 0 };
	u32 height{ 0 };

	GLFWwindow *window{ nullptr };
};
