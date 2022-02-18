#include <stdio.h>

/* (TODO, thoave01): Don't expose Vulkan backend directly. */
#include <platform/window.h>
#include <renderer/vulkan/context.h>
#include <utils/util.h>

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	Vulkan::Context ctx{};

	/* Required for GLFW. */
	ctx.addInstanceExtension("VK_KHR_surface");
	ctx.addInstanceExtension("VK_KHR_xcb_surface");
	ctx.addInstanceExtension("VK_KHR_get_physical_device_properties2");

	/* Nice to have. Should be enabled with envvar? */
	ctx.addInstanceLayer("VK_LAYER_KHRONOS_validation");

	/* I don't really need anything yet. */
	ctx.addDeviceExtension("VK_KHR_maintenance2");
	ctx.addDeviceExtension("VK_KHR_multiview");
	ctx.addDeviceExtension("VK_KHR_create_renderpass2");
	ctx.addDeviceExtension("VK_KHR_swapchain");

	ctx.build();

	// while (window.step())
	// {
	// 	;
	// }
}
