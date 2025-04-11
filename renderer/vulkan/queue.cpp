#include <renderer/vulkan/queue.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

void queue::build(device &device)
{
	vkGetDeviceQueue(device.m_logical.m_handle, *device.m_physical.m_queue_family.m_all, 0, &m_handle);
}

void queue::submit(command_buffer &command_buffer, semaphore &wait_semaphore, semaphore &signal_semaphore, fence &fence)
{
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { wait_semaphore.m_handle };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer.m_handle;

	VkSemaphore signal_semaphores[] = { signal_semaphore.m_handle };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	VULKAN_ASSERT_SUCCESS(vkQueueSubmit(m_handle, 1, &submit_info, fence.m_handle));
}

void queue::present(semaphore &wait_semaphore, wsi &wsi, u32 image_index)
{
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore wait_semaphores[] = { wait_semaphore.m_handle };
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = wait_semaphores;

	VkSwapchainKHR swapchains[] = { wsi.m_swapchain.m_handle };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr;
	vkQueuePresentKHR(m_handle, &present_info);
}

void queue::wait()
{
	vkQueueWaitIdle(m_handle);
}

} /* namespace vulkan */
