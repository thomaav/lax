#include <renderer/vulkan/util.h>
#include <renderer/vulkan/wsi.h>
#include <utils/util.h>

namespace
{
} /* namespace */

namespace Vulkan
{

void WSI::buildSurface(Instance &instance)
{
	window.handle.init(window.width, window.height);
	VULKAN_ASSERT_SUCCESS(window.handle.createVulkanSurface(instance.handle, surface.handle));

	surface.instance = &instance;
}

void WSI::destroySurface()
{
	vkDestroySurfaceKHR(surface.instance->handle, surface.handle, nullptr);
}

void WSI::buildSwapchain(Device &device)
{
	/* Create swapchain. */
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical.handle, surface.handle, &capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.physical.handle, surface.handle, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.physical.handle, surface.handle, &formatCount, availableFormats.data());

	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.physical.handle, surface.handle, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.physical.handle, surface.handle, &presentModeCount,
	                                          availablePresentModes.data());

	if (availableFormats.empty() || availablePresentModes.empty())

	{
		terminate("No suiteable swapchain formats");
	}

	if (availablePresentModes.empty())

	{
		terminate("No suiteable swapchain present modes");
	}

	/* Choose swapchain surface format. */
	VkSurfaceFormatKHR surfaceFormat{};

	bool found = false;
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
		    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surfaceFormat = availableFormat;
			found = true;
		}
	}

	if (!found)
	{
		terminate("No suitable swapchain format found");
	}

	/* Choose swapchain presentation mode. */
	VkPresentModeKHR presentMode{};

	found = false;
	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = availablePresentMode;
			found = true;
		}
	}

	if (!found)
	{
		presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	/* Choose swapchain extent. */
	int width{};
	int height{};
	window.handle.getFramebufferSize(width, height);

	VkExtent2D extent = {
		static_cast<u32>(width),
		static_cast<u32>(height),
	};

	extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	/* Swapchain image count. */
	u32 imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	/* Create swapchain. */
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface.handle;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	/* (TODO, thoave01): We are assuming a single queue. */
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VULKAN_ASSERT_SUCCESS(vkCreateSwapchainKHR(device.logical.handle, &createInfo, nullptr, &swapchain.handle));

	/* Initialize swapchain object. */
	{
		swapchain.device = &device;
		swapchain.format = surfaceFormat.format;
		swapchain.extent = extent;

		/* Initialize images. */
		vkGetSwapchainImagesKHR(swapchain.device->logical.handle, swapchain.handle, &imageCount, nullptr);
		swapchain.images.resize(imageCount);
		vkGetSwapchainImagesKHR(swapchain.device->logical.handle, swapchain.handle, &imageCount,
		                        swapchain.images.data());

		/* Initalize image views. */
		swapchain.imageViews.resize(swapchain.images.size());

		for (size_t i = 0; i < swapchain.images.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchain.images[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapchain.format;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VULKAN_ASSERT_SUCCESS(
			    vkCreateImageView(swapchain.device->logical.handle, &createInfo, nullptr, &swapchain.imageViews[i]));
		}
	}
}

void WSI::destroySwapchain()
{
	for (auto &imageView : swapchain.imageViews)
	{
		vkDestroyImageView(swapchain.device->logical.handle, imageView, nullptr);
	}

	vkDestroySwapchainKHR(swapchain.device->logical.handle, swapchain.handle, nullptr);
}

} /* namespace Vulkan */
