#include "semaphore.h"
#include "util.h"

namespace vulkan
{

semaphore::~semaphore()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(m_device_handle, m_handle, nullptr);
	}
}

void semaphore::build(device &device)
{
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VULKAN_ASSERT_SUCCESS(vkCreateSemaphore(device.m_logical.m_handle, &semaphore_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
