#include <renderer/vulkan/queue.h>
#include <renderer/vulkan/util.h>

namespace Vulkan
{

void Queue::build(Device &device)
{
	/* (TODO, thoave01): Split presentation and graphics queue? Etc. */
	vkGetDeviceQueue(device.logical.handle, *device.physical.queueFamily.all, 0, &handle);
}

/* (TODO, thoave01): Multiple command buffers. */
void Queue::submit(CommandBuffer &commandBuffer, Semaphore &waitSemaphore, Semaphore &signalSemaphore)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { waitSemaphore.handle };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer.handle;

	VkSemaphore signalSemaphores[] = { signalSemaphore.handle };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VULKAN_ASSERT_SUCCESS(vkQueueSubmit(handle, 1, &submitInfo, VK_NULL_HANDLE));
}

void Queue::present(Semaphore &waitSemaphore, WSI &wsi, u32 imageIndex)
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore waitSemaphores[] = { waitSemaphore.handle };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = waitSemaphores;

	VkSwapchainKHR swapchains[] = { wsi.swapchain.handle };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(handle, &presentInfo);
}

void Queue::wait()
{
	vkQueueWaitIdle(handle);
}

} /* namespace Vulkan */
