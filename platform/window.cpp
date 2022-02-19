#include <cassert>

#include <platform/window.h>
#include <utils/util.h>

namespace
{

void defaultKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	UNUSED(scancode);
	UNUSED(mods);

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

} /* namespace */

glfwWindow::glfwWindow()
{
}

glfwWindow::~glfwWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void glfwWindow::init(u32 width, u32 height)
{
	this->width = width;
	this->height = height;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, "window", nullptr, nullptr);

	glfwSetKeyCallback(window, defaultKeyCallback);
}

bool glfwWindow::step()
{
	glfwPollEvents();

	if (glfwWindowShouldClose(window))
	{
		return false;
	}

	return true;
}

VkResult glfwWindow::createVulkanSurface(VkInstance instance, VkSurfaceKHR &surface)
{
	return glfwCreateWindowSurface(instance, window, nullptr, &surface);
}

void glfwWindow::getFramebufferSize(int &width, int &height)
{
	glfwGetFramebufferSize(window, &width, &height);
}
