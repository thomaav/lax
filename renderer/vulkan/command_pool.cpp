#include <renderer/vulkan/command_pool.h>
#include <renderer/vulkan/util.h>

namespace Vulkan
{

void CommandPool::build(Device &device)
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = *device.physical.queueFamily.all;
	poolInfo.flags = 0;

	VULKAN_ASSERT_SUCCESS(vkCreateCommandPool(device.logical.handle, &poolInfo, nullptr, &handle));

	this->device = device.logical.handle;
}

void CommandPool::destroy()
{
	vkDestroyCommandPool(device, handle, nullptr);
}

} /* namespace Vulkan */
