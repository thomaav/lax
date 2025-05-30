#pragma once

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <third_party/vma/include/vk_mem_alloc.h>
#pragma clang diagnostic pop
// clang-format on

#include "device.h"

namespace vulkan
{

class buffer
{
public:
	buffer() = default;
	~buffer();

	buffer(const buffer &) = delete;
	buffer operator=(const buffer &) = delete;

	buffer(buffer &&o) noexcept;
	buffer &operator=(buffer &&o) noexcept;

	void build(VmaAllocator allocator, VkBufferUsageFlags usage, VkDeviceSize size);
	void fill(const void *data, size_t size);

	VkBuffer m_handle = {};
	VkDeviceSize m_size = 0;

private:
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VmaAllocation m_allocation = VK_NULL_HANDLE;
};

} /* namespace vulkan */
