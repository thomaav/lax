#pragma once

#include <third_party/volk/volk.h>

#include <renderer/vulkan/device.h>

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
