#include <iostream>

/* (TODO, thoave01): Don't expose Vulkan backend directly. */
#include "context.h"
#include "window.h"

int main(int argc, char *argv[])
{
	Renderer::glfwWindow window{800, 600};
	window.init();

	Vulkan::Context ctx{};
	ctx.init();

	while (window.step())
	{
		;
	}
}
