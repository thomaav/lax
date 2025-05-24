#pragma once

#include <vector>

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop
// clang-format on

#include <utils/type.h>

class glfw_window
{
public:
	glfw_window() = default;
	~glfw_window();

	glfw_window(const glfw_window &) = delete;
	glfw_window operator=(const glfw_window &) = delete;

	/* Window stuff. */
	void init();
	bool step();
	VkResult create_vulkan_surface(VkInstance instance, VkSurfaceKHR &surface);
	void get_framebuffer_size(int &width, int &height);
	std::vector<const char *> get_required_surface_extensions();

	GLFWwindow *m_window = nullptr;

private:
};
