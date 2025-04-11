#include <renderer/vulkan/command_pool.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

command_pool::~command_pool()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(m_device_handle, m_handle, nullptr);
	}
}

void command_pool::build(device &device)
{
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = *device.m_physical.m_queue_family.m_all;
	pool_info.flags = 0;

	VULKAN_ASSERT_SUCCESS(vkCreateCommandPool(device.m_logical.m_handle, &pool_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
