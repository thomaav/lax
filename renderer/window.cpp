#include "window.h"

namespace
{

void defaultKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

}

namespace Renderer
{

glfwWindow::glfwWindow(u32 width, u32 height)
	: width(width)
	, height(height)
{
}

glfwWindow::~glfwWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void glfwWindow::init()
{
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

}
