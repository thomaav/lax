#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>

#include <platform/window.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace Vulkan
{

Context::~Context()
{
	/* Destroy device. */
	wsi.destroySwapchain();
	device.destroy();

	/* Destroy instance. */
	wsi.destroySurface();
	instance.destroy();
}

void Context::addInstanceExtension(const char *extension)
{
	instance.addExtension(extension);
}

void Context::addInstanceLayer(const char *layer)
{
	instance.addLayer(layer);
}

void Context::addDeviceExtension(const char *extension)
{
	/* (TODO, thoave01): Assert if we already built the context. */
	device.addExtension(extension);
}

/* (TODO, thoave01): Blabla platform window can be templated. Fuck it use glfwWindow. */
void Context::build()
{
	/* Instance initialization. */
	{
		/* (TODO, thoave01): Some error handling? */
		instance.build();

		/* (TODO, thoave01): WSI should be optional. */
		/* (TODO, thoave01): WSI width/height or something. */
		wsi.buildSurface(instance);
	}

	/* Device initialization. */
	{
		device.build(instance, wsi.surface.handle);
		wsi.buildSwapchain(device);
		queue.build(device);
	}
}

} /* namespace Vulkan */
