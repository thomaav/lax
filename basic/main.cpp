#include <renderer/vulkan/context.h>
#include <utils/util.h>

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	vulkan::context ctx{};

	/* (TODO, thoave01): Get GLFW extension requirements from platform. */
	ctx.add_instance_extension("VK_KHR_surface");
	ctx.add_instance_extension("VK_KHR_xcb_surface");
	ctx.add_instance_extension("VK_KHR_get_physical_device_properties2");

	ctx.add_device_extension("VK_KHR_swapchain");
	ctx.add_device_extension("VK_EXT_shader_object");

	ctx.build();
	ctx.backend_test();
}
