#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <vma/vk_mem_alloc.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/instance.h>

namespace vulkan
{

class resource_allocator
{
public:
	resource_allocator() = default;
	~resource_allocator();

	resource_allocator(const resource_allocator &) = delete;
	resource_allocator operator=(const resource_allocator &) = delete;

	void build(instance &instance, device &device);
	void allocate_buffer(buffer &buffer, VkBufferUsageFlags usage, VkDeviceSize size);

private:
	instance *m_instance = nullptr;
	device *m_device = nullptr;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
};

} /* namespace vulkan */
