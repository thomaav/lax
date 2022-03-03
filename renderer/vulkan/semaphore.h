#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class Semaphore
{
public:
	Semaphore() = default;
	~Semaphore();

	Semaphore(const Semaphore &) = delete;
	Semaphore operator=(const Semaphore &) = delete;

	void build(Device &device);

	VkSemaphore handle{};

private:
	VkDevice device{};
};

} /* namespace Vulkan */
