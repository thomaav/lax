#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/util.h>

namespace Vulkan
{

void CommandBuffer::build(Device &device, CommandPool &commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool.handle;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VULKAN_ASSERT_SUCCESS(vkAllocateCommandBuffers(device.logical.handle, &allocInfo, &handle));

	this->device = device.logical.handle;
	this->commandPool = commandPool.handle;
}

void CommandBuffer::begin()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VULKAN_ASSERT_SUCCESS(vkBeginCommandBuffer(handle, &beginInfo));
}

void CommandBuffer::end()
{
	VULKAN_ASSERT_SUCCESS(vkEndCommandBuffer(handle));
}

} /* namespace Vulkan */
