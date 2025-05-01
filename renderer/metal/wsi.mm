#include <Metal/Metal.hpp>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/QuartzCore.hpp>

#include <renderer/metal/wsi.h>

namespace metal
{

void wsi::build(glfw_window &window, MTL::Device *metal_device)
{
	NSWindow *ns_window = glfwGetCocoaWindow(window.m_window);
	CAMetalLayer *metal_layer = [CAMetalLayer layer];
	metal_layer.device = (__bridge id<MTLDevice>)metal_device;
    metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    ns_window.contentView.layer = metal_layer;
    ns_window.contentView.wantsLayer = YES;

	m_ns_window = ns_window;
	m_metal_layer = metal_layer;
}

id wsi::get_next_drawable()
{
	CAMetalLayer *metal_layer = (__bridge CAMetalLayer*)m_metal_layer;
	return [metal_layer nextDrawable];
}

}
