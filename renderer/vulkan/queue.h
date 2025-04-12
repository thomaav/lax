#pragma once

#include <third_party/volk/volk.h>

#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/fence.h>
#include <renderer/vulkan/semaphore.h>
#include <renderer/vulkan/wsi.h>

namespace vulkan
{

class queue
{
public:
	queue() = default;
	~queue() = default;

	queue(const queue &) = delete;
	queue operator=(const queue &) = delete;

	void build(device &device);
	void submit(command_buffer &command_buffer, semaphore &wait_semaphore, semaphore &signal_semaphore, fence &fence);
	void present(semaphore &signal_semaphore, wsi &wsi, u32 image_index);
	void wait();

	VkQueue m_handle = {};

private:
};

} /* namespace vulkan */
