#pragma once

#include <vector>

#include <third_party/volk/volk.h>

#include <platform/window.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/queue.h>
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

private:
	glfw_window m_window = {};
	instance m_instance = {};
	device m_device = {};
	wsi m_wsi = {};
	queue m_queue = {};
};

} /* namespace vulkan */
