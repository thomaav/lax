#pragma once

#include <third_party/volk/volk.h>

#include <renderer/vulkan/device.h>

namespace vulkan
{

class fence
{
public:
	fence() = default;
	~fence();

	fence(const fence &) = delete;
	fence operator=(const fence &) = delete;

	void build(device &device);
	void reset();
	void wait();

	VkFence m_handle = {};

private:
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
