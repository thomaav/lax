#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class Fence
{
public:
	Fence() = default;
	~Fence();

	Fence(const Fence &) = delete;
	Fence operator=(const Fence &) = delete;

	void build(Device &device);

	void reset();

	void wait();

	VkFence handle{};

private:
	VkDevice device{};
};

} /* namespace Vulkan */
