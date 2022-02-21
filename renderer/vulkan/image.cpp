#include <renderer/vulkan/image.h>
#include <renderer/vulkan/util.h>

namespace Vulkan
{

ImageView::ImageView(Image &image)
    : image{ image }
{
}

void ImageView::build(Device &device)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image.handle;

	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = image.format;

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VULKAN_ASSERT_SUCCESS(vkCreateImageView(device.logical.handle, &createInfo, nullptr, &handle));

	this->device = device.logical.handle;
}

void ImageView::destroy()
{
	vkDestroyImageView(device, handle, nullptr);
}

} /* namespace Vulkan */
