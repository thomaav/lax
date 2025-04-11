#include <algorithm>

#include <renderer/vulkan/util.h>
#include <renderer/vulkan/wsi.h>
#include <utils/util.h>

namespace vulkan
{

void wsi::build_surface(instance &instance)
{
	window.m_handle.init(window.m_width, window.m_height);
	VULKAN_ASSERT_SUCCESS(window.m_handle.create_vulkan_surface(instance.m_handle, m_surface.handle));

	m_surface.m_instance = &instance;
}

void wsi::destroy_surface()
{
	vkDestroySurfaceKHR(m_surface.m_instance->m_handle, m_surface.handle, nullptr);
}

void wsi::build_swapchain(device &device)
{
	/* Create swapchain. */
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.m_physical.m_handle, m_surface.handle, &capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physical.m_handle, m_surface.handle, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physical.m_handle, m_surface.handle, &formatCount,
	                                     availableFormats.data());

	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physical.m_handle, m_surface.handle, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physical.m_handle, m_surface.handle, &presentModeCount,
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
	window.m_handle.get_framebuffer_size(width, height);

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
	createInfo.surface = m_surface.handle;

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

	VULKAN_ASSERT_SUCCESS(vkCreateSwapchainKHR(device.m_logical.m_handle, &createInfo, nullptr, &m_swapchain.m_handle));

	/* Initialize swapchain object. */
	{
		m_swapchain.m_device = &device;
		m_swapchain.m_format = surfaceFormat.format;
		m_swapchain.m_extent = extent;

		/* Initialize images. */
		vkGetSwapchainImagesKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle, &imageCount, nullptr);
		m_swapchain.m_images.resize(imageCount);
		vkGetSwapchainImagesKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle, &imageCount,
		                        m_swapchain.m_images.data());

		/* Initalize image views. */
		for (size_t i = 0; i < m_swapchain.m_images.size(); i++)
		{
			std::unique_ptr<image> image_ = std::make_unique<image>();
			image_->m_handle = m_swapchain.m_images[i];
			image_->m_format = m_swapchain.m_format;
			image_->m_width = m_swapchain.m_extent.width;
			image_->m_height = m_swapchain.m_extent.height;
			m_swapchain.m_vulkan_images.emplace_back(std::move(image_));

			std::unique_ptr<image_view> imageView = std::make_unique<image_view>(*m_swapchain.m_vulkan_images[i]);
			imageView->build(*m_swapchain.m_device);
			m_swapchain.m_image_views.emplace_back(std::move(imageView));
		}
	}
}

void wsi::destroy_swapchain()
{
	for (auto &imageView : m_swapchain.m_image_views)
	{
		imageView->destroy();
	}

	vkDestroySwapchainKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle, nullptr);
}

void wsi::acquire_image(semaphore &signalSemaphore, u32 *imageIndex)
{
	VULKAN_ASSERT_SUCCESS(vkAcquireNextImageKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle,
	                                            UINT64_MAX, signalSemaphore.m_handle, VK_NULL_HANDLE, imageIndex));
}

} /* namespace vulkan */
