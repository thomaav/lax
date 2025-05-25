#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <platform/window.h>
#include <utils/type.h>

#include "device.h"
#include "instance.h"
#include "queue.h"
#include "resource_allocator.h"
#include "wsi.h"

namespace vulkan
{

class context
{
public:
	context() = default;
	~context() = default;

	context(const context &) = delete;
	context operator=(const context &) = delete;

	void add_instance_extension(const char *extension);
	void add_device_extension(const char *extension);

	void build();

	command_buffer &begin_frame();
	void end_frame(texture &frame);

	glfw_window m_window = {};
	instance m_instance = {};
	device m_device = {};
	wsi m_wsi = {};
	queue m_queue = {};
	resource_allocator m_resource_allocator = {};

	command_pool m_command_pool = {};
	command_buffer m_command_buffer = {};
	semaphore m_image_available_semaphore = {};
	semaphore m_render_finished_semaphore = {};

private:
};

} /* namespace vulkan */
