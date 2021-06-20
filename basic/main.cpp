#include <iostream>

#include "window.h"

int main(int argc, char *argv[])
{
	glfwWindow window{800, 600};
	window.init();

	while (window.step())
	{
		;
	}
}
