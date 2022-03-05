#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/semaphore.h>
#include <renderer/vulkan/wsi.h>

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

	void submit(CommandBuffer &commandBuffer, Semaphore &waitSemaphore, Semaphore &signalSemaphore);

	void present(Semaphore &signalSemaphore, WSI &wsi, u32 imageIndex);

	void wait();

private:
};

} /* namespace Vulkan */
