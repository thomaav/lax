#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <vulkan/vulkan_profiles.hpp>
#pragma clang diagnostic pop

#include <platform/window.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/queue.h>
#include <renderer/vulkan/resource_allocator.h>
#include <renderer/vulkan/wsi.h>
#include <utils/type.h>

namespace vulkan
{

class context
{
public:
	context() = default;
	~context();

	context(const context &) = delete;
	context operator=(const context &) = delete;

	void add_instance_extension(const char *extension);
	void add_device_extension(const char *extension);

	void build();
	void backend_test();

	/* (TODO, thoave01): Hide this again. */
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
