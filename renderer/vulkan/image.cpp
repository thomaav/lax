#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/image.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

image::~image()
{
	if (VK_NULL_HANDLE != m_handle && !m_external_image)
	{
		vmaDestroyImage(m_allocator, m_handle, m_allocation);
	}
}

void image::build_external_image(VkImage handle, VkFormat format, u32 width, u32 height)
{
	m_external_image = true;
	m_handle = handle;
	m_format = format;
	m_width = width;
	m_height = height;
	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void image::build(VmaAllocator allocator, VkFormat format, VkImageUsageFlags usage, u32 width, u32 height)
{
	m_allocator = allocator;
	m_external_image = false;
	m_format = format;
	m_width = width;
	m_height = height;
	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	const VkExtent3D extent = {
		.width = width,   //
		.height = height, //
		.depth = 1        //
	};
	const VkImageCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //
		.pNext = nullptr,                             //
		.flags = 0,                                   //
		.imageType = VK_IMAGE_TYPE_2D,                //
		.format = format,                             //
		.extent = extent,                             //
		.mipLevels = 1,                               //
		.arrayLayers = 1,                             //
		.samples = VK_SAMPLE_COUNT_1_BIT,             //
		.tiling = VK_IMAGE_TILING_LINEAR,             //
		.usage = usage,                               //
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,     //
		.queueFamilyIndexCount = 0,                   //
		.pQueueFamilyIndices = nullptr,               //
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,   //
	};

	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

	VULKAN_ASSERT_SUCCESS(vmaCreateImage(allocator, &create_info, &alloc_info, &m_handle, &m_allocation, nullptr));
}

void image::transition_layout(context &context, VkImageLayout old_layout, VkImageLayout new_layout)
{
	command_buffer command_buffer = {};
	command_buffer.build(context.m_device, context.m_command_pool);
	command_buffer.begin();
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_handle;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		vkCmdPipelineBarrier(command_buffer.m_handle, 0, 0, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}
	command_buffer.end();
	context.m_queue.submit_and_wait(command_buffer);
	m_layout = new_layout;
}

/* (TODO, thoave01): There are multiple submissions in here. Could use VK_EXT_host_image_copy? */
void image::fill(context &context, const void *data, size_t size)
{
	VkImageLayout old_layout = m_layout;
	if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL != m_layout)
	{
		transition_layout(context, old_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	buffer staging_buffer = {};
	context.m_resource_allocator.allocate_buffer(staging_buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);
	staging_buffer.fill(data, size);

	command_buffer command_buffer = {};
	command_buffer.build(context.m_device, context.m_command_pool);
	command_buffer.begin();
	{
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { m_width, m_height, 1 };
		vkCmdCopyBufferToImage(command_buffer.m_handle, staging_buffer.m_handle, m_handle,
		                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	command_buffer.end();
	context.m_queue.submit_and_wait(command_buffer);

	if (m_layout != old_layout && old_layout != VK_IMAGE_LAYOUT_UNDEFINED)
	{
		transition_layout(context, m_layout, old_layout);
	}
}

image_view::~image_view()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkDestroyImageView(m_device_handle, m_handle, nullptr);
	}
}

image_view::image_view(image &image)
    : m_image(image)
{
}

void image_view::build(device &device)
{
	VkImageViewCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = m_image.m_handle;

	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = m_image.m_format;

	create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = 1;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;

	VULKAN_ASSERT_SUCCESS(vkCreateImageView(device.m_logical.m_handle, &create_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
