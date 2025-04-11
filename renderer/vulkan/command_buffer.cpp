#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

void command_buffer::build(device &device, command_pool &command_pool)
{
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool.m_handle;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	VULKAN_ASSERT_SUCCESS(vkAllocateCommandBuffers(device.m_logical.m_handle, &alloc_info, &m_handle));

	m_device_handle = device.m_logical.m_handle;
	m_command_pool_handle = command_pool.m_handle;
}

void command_buffer::begin()
{
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = nullptr;

	VULKAN_ASSERT_SUCCESS(vkBeginCommandBuffer(m_handle, &begin_info));
}

void command_buffer::end()
{
	VULKAN_ASSERT_SUCCESS(vkEndCommandBuffer(m_handle));
}

} /* namespace vulkan */
