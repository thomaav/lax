#pragma once

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop
// clang-format on

class input
{
public:
	input() = default;
	~input();

	input(const input &) = delete;
	input operator=(const input &) = delete;

	static void register_window(GLFWwindow *window);

	static bool is_key_pressed(int key);
	static bool is_mouse_button_pressed(int button);
	static void get_mouse_position(double &x, double &y);

private:
	static GLFWwindow *m_window;
};
