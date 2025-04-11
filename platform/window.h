#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <utils/type.h>

class glfw_window
{
public:
	glfw_window() = default;
	~glfw_window();

	glfw_window(const glfw_window &) = delete;
	glfw_window operator=(const glfw_window &) = delete;

	void init(u32 width, u32 height);
	bool step();
	VkResult create_vulkan_surface(VkInstance instance, VkSurfaceKHR &surface);
	void get_framebuffer_size(int &width, int &height);

private:
	u32 m_width{ 0 };
	u32 m_height{ 0 };
	GLFWwindow *m_window{ nullptr };
};
