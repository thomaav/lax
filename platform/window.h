#pragma once

#include <vector>

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <GLFW/glfw3.h>
#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#endif
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
	void init(u32 width, u32 height);
	bool step();
	VkResult create_vulkan_surface(VkInstance instance, VkSurfaceKHR &surface);
	void get_framebuffer_size(int &width, int &height);
	std::vector<const char *> get_required_surface_extensions();

	/* Input handling. */
	bool is_key_pressed(int key);
	bool is_mouse_button_pressed(int button);
	void get_mouse_position(double &x, double &y);

	/* (TODO, thoave01): private; this is just for Metal testing. */
	/* (TODO, thoave01): We should just expose get_cocoa_window under LAX_METAL. */
	GLFWwindow *m_window = nullptr;

private:
	u32 m_width = 0;
	u32 m_height = 0;
};
