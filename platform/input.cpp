#include "input.h"

GLFWwindow *input::m_window = nullptr;

void input::register_window(GLFWwindow *window)
{
	m_window = window;
}

bool input::is_key_pressed(int key)
{
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool input::is_mouse_button_pressed(int button)
{
	return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void input::get_mouse_position(double &x, double &y)
{
	glfwGetCursorPos(m_window, &x, &y);
}
