#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace vulkan
{

buffer::~buffer()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vmaDestroyBuffer(m_allocator, m_handle, m_allocation);
	}
}

void buffer::build(VmaAllocator allocator, VkBufferUsageFlags usage, VkDeviceSize size)
{
	m_allocator = allocator;

	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = size;
	create_info.usage = usage;

	VmaAllocationCreateInfo alloc_create_info = {};
	alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
	alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

	VULKAN_ASSERT_SUCCESS(
	    vmaCreateBuffer(allocator, &create_info, &alloc_create_info, &m_handle, &m_allocation, nullptr));
}

void buffer::fill(const void *data, size_t size)
{
	void *content = nullptr;
	vmaMapMemory(m_allocator, m_allocation, &content);
	memcpy(content, data, size);
	vmaUnmapMemory(m_allocator, m_allocation);
}

} /* namespace vulkan */
