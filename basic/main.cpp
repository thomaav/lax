#include <renderer/vulkan/context.h>
#include <utils/util.h>

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	vulkan::context ctx{};

	/* Required for GLFW. */
	ctx.add_instance_extension("VK_KHR_surface");
	ctx.add_instance_extension("VK_KHR_xcb_surface");
	ctx.add_instance_extension("VK_KHR_get_physical_device_properties2");

	/* I don't really need anything yet. */
	ctx.add_device_extension("VK_KHR_maintenance2");
	ctx.add_device_extension("VK_KHR_multiview");
	ctx.add_device_extension("VK_KHR_create_renderpass2");
	ctx.add_device_extension("VK_KHR_swapchain");

	ctx.build();

	// while (window.step())
	// {
	// 	;
	// }
}
