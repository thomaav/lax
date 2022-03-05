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
		for (size_t i = 0; i < swapchain.images.size(); i++)
		{
			/* (TODO, thoave01): Fix this. */
			std::unique_ptr<Image> image = std::make_unique<Image>();
			image->handle = swapchain.images[i];
			image->format = swapchain.format;
			image->width = swapchain.extent.width;
			image->height = swapchain.extent.height;
			swapchain.vulkanImages.emplace_back(std::move(image));

			std::unique_ptr<ImageView> imageView = std::make_unique<ImageView>(*swapchain.vulkanImages[i]);
			imageView->build(*swapchain.device);
			swapchain.imageViews.emplace_back(std::move(imageView));
		}
	}
}

void WSI::destroySwapchain()
{
	for (auto &imageView : swapchain.imageViews)
	{
		imageView->destroy();
	}

	vkDestroySwapchainKHR(swapchain.device->logical.handle, swapchain.handle, nullptr);
}

void WSI::acquireImage(Semaphore &signalSemaphore, u32 *imageIndex)
{
	VULKAN_ASSERT_SUCCESS(vkAcquireNextImageKHR(swapchain.device->logical.handle, swapchain.handle, UINT64_MAX,
	                                            signalSemaphore.handle, VK_NULL_HANDLE, imageIndex));
}

} /* namespace Vulkan */
