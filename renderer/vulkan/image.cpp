#include <renderer/vulkan/image.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

image_view::~image_view()
{
	vkDestroyImageView(m_device_handle, m_handle, nullptr);
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
