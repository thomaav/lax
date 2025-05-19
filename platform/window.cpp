#include <cassert>

#include <platform/window.h>
#include <utils/util.h>

static void default_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	UNUSED(scancode);
	UNUSED(mods);

	/* Exiting. */
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

glfw_window::~glfw_window()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void glfw_window::init()
{

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	m_window = glfwCreateWindow(mode->width, mode->height, "window", glfwGetPrimaryMonitor(), nullptr);

	glfwSetKeyCallback(m_window, default_key_callback);
}

bool glfw_window::step()
{
	glfwPollEvents();

	if (glfwWindowShouldClose(m_window))
	{
		return false;
	}

	return true;
}

VkResult glfw_window::create_vulkan_surface(VkInstance instance, VkSurfaceKHR &surface)
{
	return glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
}

void glfw_window::get_framebuffer_size(int &width, int &height)
{
	glfwGetFramebufferSize(m_window, &width, &height);
}

std::vector<const char *> glfw_window::get_required_surface_extensions()
{
	u32 extension_count = 0;
	const char **extensions = glfwGetRequiredInstanceExtensions(&extension_count);
	return { extensions, extensions + extension_count };
}

bool glfw_window::is_key_pressed(int key)
{
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool glfw_window::is_mouse_button_pressed(int button)
{
	return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void glfw_window::get_mouse_position(double &x, double &y)
{
	glfwGetCursorPos(m_window, &x, &y);
}
