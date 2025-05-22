#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include "device.h"

namespace vulkan
{

class semaphore
{
public:
	semaphore() = default;
	~semaphore();

	semaphore(const semaphore &) = delete;
	semaphore operator=(const semaphore &) = delete;

	void build(device &device);

	VkSemaphore m_handle = {};

private:
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
