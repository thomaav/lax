#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <vma/vk_mem_alloc.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/device.h>

namespace vulkan
{

class buffer
{
public:
	buffer() = default;
	~buffer();

	buffer(const buffer &) = delete;
	buffer operator=(const buffer &) = delete;

	void build(VmaAllocator allocator, VkBufferUsageFlags usage, VkDeviceSize size);
	void fill(const void *data, size_t size);

	VkBuffer m_handle = {};

private:
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VmaAllocation m_allocation = VK_NULL_HANDLE;
};

} /* namespace vulkan */
