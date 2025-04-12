#pragma once

#include <third_party/volk/volk.h>

#include <renderer/vulkan/device.h>

namespace vulkan
{

class command_pool
{
public:
	command_pool() = default;
	~command_pool();

	command_pool(const command_pool &) = delete;
	command_pool operator=(const command_pool &) = delete;

	void build(device &device);

	VkCommandPool m_handle = {};

private:
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
