#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/command_pool.h>
#include <renderer/vulkan/device.h>

namespace vulkan
{

class command_buffer
{
public:
	command_buffer() = default;
	~command_buffer() = default;

	command_buffer(const command_buffer &) = delete;
	command_buffer operator=(const command_buffer &) = delete;

	void build(device &device, command_pool &command_pool);
	void begin();
	void end();

	VkCommandBuffer m_handle = {};

private:
	VkDevice m_device_handle = {};
	VkCommandPool m_command_pool_handle = {};
};

} /* namespace vulkan */
