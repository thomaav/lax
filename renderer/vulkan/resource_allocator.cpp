#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

#include <renderer/vulkan/image.h>
#include <renderer/vulkan/resource_allocator.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

resource_allocator::~resource_allocator()
{
	if (VK_NULL_HANDLE != m_allocator)
	{
		vmaDestroyAllocator(m_allocator);
	}
}

void resource_allocator::build(instance &instance, device &device)
{
	m_instance = &instance;
	m_device = &device;

	VmaVulkanFunctions vulkan_functions = {};
	vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	VmaAllocatorCreateInfo alloc_info = {};
	alloc_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	alloc_info.vulkanApiVersion = VK_API_VERSION_1_3;
	alloc_info.physicalDevice = m_device->m_physical.m_handle;
	alloc_info.device = m_device->m_logical.m_handle;
	alloc_info.instance = m_instance->m_handle;
	alloc_info.pVulkanFunctions = &vulkan_functions;
	vmaCreateAllocator(&alloc_info, &m_allocator);
}

buffer resource_allocator::allocate_buffer(VkBufferUsageFlags usage, VkDeviceSize size)
{
	buffer buffer = {};
	buffer.build(m_allocator, usage, size);
	return buffer;
}

void resource_allocator::allocate_buffer(buffer &buffer, VkBufferUsageFlags usage, VkDeviceSize size)
{
	buffer.build(m_allocator, usage, size);
}

} /* namespace vulkan */
