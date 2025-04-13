#pragma once

#include <vector>

// clang-format off
#include <third_party/volk/volk.h>
#include <GLFW/glfw3.h>
// clang-format on

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
	std::vector<const char *> get_required_surface_extensions();

private:
	u32 m_width = 0;
	u32 m_height = 0;
	GLFWwindow *m_window = nullptr;
};
