#include <renderer/vulkan/fence.h>
#include <renderer/vulkan/util.h>

namespace Vulkan
{

Fence::~Fence()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyFence(device, handle, nullptr);
	}
}

void Fence::build(Device &device)
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VULKAN_ASSERT_SUCCESS(vkCreateFence(device.logical.handle, &fenceInfo, nullptr, &handle));

	this->device = device.logical.handle;
}

void Fence::reset()
{
	VULKAN_ASSERT_SUCCESS(vkResetFences(device, 1, &handle));
}

void Fence::wait()
{
	VULKAN_ASSERT_SUCCESS(vkWaitForFences(device, 1, &handle, VK_TRUE, UINT64_MAX));
}

} /* namespace Vulkan */
