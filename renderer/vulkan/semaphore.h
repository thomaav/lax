#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class Semaphore
{
public:
	Semaphore() = default;
	~Semaphore() = default;

	Semaphore(const Semaphore &) = delete;
	Semaphore operator=(const Semaphore &) = delete;

	void build(Device &device);

	/* (TODO, thoave01): Make everything into destructors checking VK_NULL_HANDLE. */
	void destroy();

	VkSemaphore handle{};

private:
	VkDevice device{};
};

} /* namespace Vulkan */
