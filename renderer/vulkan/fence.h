#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

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
