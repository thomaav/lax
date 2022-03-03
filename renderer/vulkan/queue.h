#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class Queue
{
public:
	Queue() = default;
	~Queue() = default;

	Queue(const Queue &) = delete;
	Queue operator=(const Queue &) = delete;

	VkQueue handle{};

	void build(Device &device);

	void wait();

private:
};

} /* namespace Vulkan */
