#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <string_view>

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop
// clang-format on

#include <platform/window.h>
#include <utils/log.h>
#include <utils/type.h>
#include <utils/util.h>

#include "buffer.h"
#include "command_buffer.h"
#include "context.h"
#include "fence.h"
#include "pipeline.h"
#include "render_pass.h"
#include "semaphore.h"
#include "shader.h"
#include "util.h"

namespace vulkan
{

void context::add_instance_extension(const char *extension)
{
	m_instance.add_extension(extension);
}

void context::add_device_extension(const char *extension)
{
	m_device.add_extension(extension);
}

void context::build()
{
	/* Initialize loading. */
	VULKAN_ASSERT_SUCCESS(volkInitialize());

	m_device.add_extension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
	const VpProfileProperties profile_properties = {
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_NAME,        //
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_SPEC_VERSION //
	};

	/* Instance initialization. */
	m_window.init();
	m_instance.build(m_window, profile_properties);
	m_wsi.build_surface(m_window, m_instance);

	/* Device initialization. */
	m_device.build(m_instance, m_wsi.m_surface.handle, profile_properties);
	m_wsi.build_swapchain(m_device);
	m_queue.build(m_device);

	/* Resource management initialization. */
	m_resource_allocator.build(m_instance, m_device);
	m_command_pool.build(m_device);
}

} /* namespace vulkan */
