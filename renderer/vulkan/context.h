#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <vulkan/vulkan_profiles.hpp>
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

	glfw_window m_window = {};
	instance m_instance = {};
	device m_device = {};
	wsi m_wsi = {};
	queue m_queue = {};
	resource_allocator m_resource_allocator = {};
	command_pool m_command_pool = {};

private:
};

} /* namespace vulkan */
