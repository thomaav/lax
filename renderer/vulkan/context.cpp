#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <string_view>

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop
// clang-format on

#include <platform/window.h>
#include <utils/log.h>
#include <utils/type.h>
#include <utils/util.h>

#include "buffer.h"
#include "command_buffer.h"
#include "context.h"
#include "fence.h"
#include "pipeline.h"
#include "render_pass.h"
#include "semaphore.h"
#include "shader.h"
#include "util.h"

namespace vulkan
{

void context::add_instance_extension(const char *extension)
{
	m_instance.add_extension(extension);
}

void context::add_device_extension(const char *extension)
{
	m_device.add_extension(extension);
}

void context::build()
{
	/* Initialize loading. */
	VULKAN_ASSERT_SUCCESS(volkInitialize());

	m_device.add_extension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
	const VpProfileProperties profile_properties = {
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_NAME,        //
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_SPEC_VERSION //
	};

	/* Instance initialization. */
	m_window.init();
	m_instance.build(m_window, profile_properties);
	m_wsi.build_surface(m_window, m_instance);

	/* Device initialization. */
	m_device.build(m_instance, m_wsi.m_surface.handle, profile_properties);
	m_wsi.build_swapchain(m_device);
	m_queue.build(m_device);

	/* Resource management initialization. */
	m_resource_allocator.build(m_instance, m_device);
	m_command_pool.build(m_device);
	m_command_buffer.build(m_device, m_command_pool);
	m_image_available_semaphore.build(m_device);
	m_render_finished_semaphore.build(m_device);
}

command_buffer &context::begin_frame()
{
	/* Reset command pool every frame for simplicity. */
	m_command_pool.reset();

	/* Command buffer will implicitly reset on begin(). */
	m_command_buffer.begin();
	return m_command_buffer;
}

void context::end_frame(texture &frame)
{
	u32 image_idx = 0;
	m_wsi.acquire_image(m_image_available_semaphore, &image_idx);

	/* Finalize command buffer by copying input frame to swapchain. */
	{
		m_command_buffer.transition_image_layout(
		    frame.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);
		m_command_buffer.transition_image_layout(*m_wsi.m_swapchain.m_images[image_idx],
		                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0,
		                                         VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

		VkImageCopy copy_info = {};
		copy_info.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_info.srcSubresource.mipLevel = 0;
		copy_info.srcSubresource.baseArrayLayer = 0;
		copy_info.srcSubresource.layerCount = 1;
		copy_info.srcOffset = { 0, 0, 0 };
		copy_info.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_info.dstSubresource.mipLevel = 0;
		copy_info.dstSubresource.baseArrayLayer = 0;
		copy_info.dstSubresource.layerCount = 1;
		copy_info.dstOffset = { 0, 0, 0 };
		copy_info.extent = { frame.m_image.m_info.m_width, frame.m_image.m_info.m_height, 1 };
		vkCmdCopyImage(m_command_buffer.m_handle, frame.m_image.m_handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		               m_wsi.m_swapchain.m_images[image_idx]->m_handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
		               &copy_info);

		m_command_buffer.transition_image_layout(*m_wsi.m_swapchain.m_images[image_idx],
		                                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		                                         VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, 0);
	}
	m_command_buffer.end();

	vulkan::fence unused_fence = {};
	vulkan::semaphore unused_semaphore = {};
	m_queue.submit(m_command_buffer, m_image_available_semaphore, m_render_finished_semaphore, unused_fence);
	m_queue.present(m_render_finished_semaphore, m_wsi, image_idx);
	m_device.wait();
}

} /* namespace vulkan */
