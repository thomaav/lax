#include <algorithm>

#include <renderer/vulkan/util.h>
#include <renderer/vulkan/wsi.h>
#include <utils/util.h>

namespace vulkan
{

wsi::~wsi()
{
	/* Destroy swapchain. */
	if (VK_NULL_HANDLE != m_swapchain.m_handle)
	{
		vkDestroySwapchainKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle, nullptr);
	}

	/* Destroy surface. */
	if (VK_NULL_HANDLE != m_surface.handle)
	{
		vkDestroySurfaceKHR(m_surface.m_instance->m_handle, m_surface.handle, nullptr);
	}
}

void wsi::build_surface(glfw_window &window, instance &instance)
{
	VULKAN_ASSERT_SUCCESS(window.create_vulkan_surface(instance.m_handle, m_surface.handle));

	m_surface.m_instance = &instance;
	m_surface.m_window = &window;
}

void wsi::build_swapchain(device &device)
{
	/* Create swapchain. */
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.m_physical.m_handle, m_surface.handle, &capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physical.m_handle, m_surface.handle, &format_count, nullptr);
	std::vector<VkSurfaceFormatKHR> available_formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physical.m_handle, m_surface.handle, &format_count,
	                                     available_formats.data());

	u32 present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physical.m_handle, m_surface.handle, &present_mode_count,
	                                          nullptr);
	std::vector<VkPresentModeKHR> available_present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physical.m_handle, m_surface.handle, &present_mode_count,
	                                          available_present_modes.data());

	if (available_formats.empty() || available_present_modes.empty())
	{
		terminate("No suitable swapchain formats");
	}

	if (available_present_modes.empty())
	{
		terminate("No suitable swapchain present modes");
	}

	/* Choose swapchain surface format. */
	VkSurfaceFormatKHR surface_format = {};
	bool found = false;
	for (const auto &available_format : available_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
		    available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surface_format = available_format;
			found = true;
		}
	}

	if (!found)
	{
		terminate("No suitable swapchain format found");
	}

	/* Choose swapchain presentation mode. */
	VkPresentModeKHR present_mode = {};
	found = false;
	for (const auto &available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			present_mode = available_present_mode;
			found = true;
		}
	}

	if (!found)
	{
		present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	/* Choose swapchain extent. */
	int width = 0;
	int height = 0;
	m_surface.m_window->get_framebuffer_size(width, height);

	VkExtent2D extent = {
		(u32)width,
		(u32)height,
	};

	extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	/* Swapchain image count. */
	u32 image_count = capabilities.minImageCount;
	if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
	{
		image_count = capabilities.maxImageCount;
	}

	/* Create swapchain. */
	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = m_surface.handle;

	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = nullptr;

	create_info.preTransform = capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	create_info.oldSwapchain = VK_NULL_HANDLE;

	VULKAN_ASSERT_SUCCESS(
	    vkCreateSwapchainKHR(device.m_logical.m_handle, &create_info, nullptr, &m_swapchain.m_handle));

	/* Initialize swapchain object. */
	{
		m_swapchain.m_device = &device;
		m_swapchain.m_format = surface_format.format;
		m_swapchain.m_extent = extent;

		/* Initialize images. */
		vkGetSwapchainImagesKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle, &image_count, nullptr);
		m_swapchain.m_vulkan_images.resize(image_count);
		vkGetSwapchainImagesKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle, &image_count,
		                        m_swapchain.m_vulkan_images.data());

		/* Initalize image views. */
		for (size_t i = 0; i < m_swapchain.m_vulkan_images.size(); i++)
		{
			ref<image> img = make_ref<image>();
			img->build_external(m_swapchain.m_vulkan_images[i], m_swapchain.m_format, m_swapchain.m_extent.width,
			                    m_swapchain.m_extent.height);
			m_swapchain.m_images.push_back(img);

			std::unique_ptr<image_view> img_view = std::make_unique<image_view>();
			img_view->build(*m_swapchain.m_device, *m_swapchain.m_images[i]);
			m_swapchain.m_image_views.emplace_back(std::move(img_view));
		}
	}
}

void wsi::acquire_image(semaphore &semaphore, u32 *image_index)
{
	VULKAN_ASSERT_SUCCESS(vkAcquireNextImageKHR(m_swapchain.m_device->m_logical.m_handle, m_swapchain.m_handle,
	                                            UINT64_MAX, semaphore.m_handle, VK_NULL_HANDLE, image_index));
}

} /* namespace vulkan */
