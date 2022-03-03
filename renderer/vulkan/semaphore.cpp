#include <renderer/vulkan/semaphore.h>
#include <renderer/vulkan/util.h>

namespace Vulkan
{

Semaphore::~Semaphore()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(device, handle, nullptr);
	}
}

void Semaphore::build(Device &device)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VULKAN_ASSERT_SUCCESS(vkCreateSemaphore(device.logical.handle, &semaphoreInfo, nullptr, &handle));

	this->device = device.logical.handle;
}

} /* namespace Vulkan */
