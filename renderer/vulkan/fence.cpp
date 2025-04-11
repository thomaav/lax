#include <renderer/vulkan/fence.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

fence::~fence()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyFence(m_device_handle, m_handle, nullptr);
	}
}

void fence::build(device &device)
{
	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VULKAN_ASSERT_SUCCESS(vkCreateFence(device.m_logical.m_handle, &fence_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

void fence::reset()
{
	VULKAN_ASSERT_SUCCESS(vkResetFences(m_device_handle, 1, &m_handle));
}

void fence::wait()
{
	VULKAN_ASSERT_SUCCESS(vkWaitForFences(m_device_handle, 1, &m_handle, VK_TRUE, UINT64_MAX));
}

} /* namespace vulkan */
