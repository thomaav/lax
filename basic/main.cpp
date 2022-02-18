#include <stdio.h>

/* (TODO, thoave01): Don't expose Vulkan backend directly. */
#include <renderer/vulkan/context.h>
#include <renderer/window.h>

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Renderer::glfwWindow window{ 800, 600 };
	window.init();

	Vulkan::Context ctx{};
	ctx.build();

	printf("context initialized successfully\n");

	// while (window.step())
	// {
	// 	;
	// }
}
