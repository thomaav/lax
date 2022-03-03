#include <renderer/vulkan/semaphore.h>
#include <renderer/vulkan/util.h>

namespace Vulkan
{

void Semaphore::build(Device &device)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VULKAN_ASSERT_SUCCESS(vkCreateSemaphore(device.logical.handle, &semaphoreInfo, nullptr, &handle));

	this->device = device.logical.handle;
}

void Semaphore::destroy()
{
	vkDestroySemaphore(device, handle, nullptr);
}

} /* namespace Vulkan */
