#pragma once

#include <memory>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <platform/window.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/image.h>
#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/semaphore.h>
#include <utils/util.h>

namespace vulkan
{

class wsi
{
public:
	wsi() = default;
	~wsi();

	wsi(const wsi &) = delete;
	wsi operator=(const wsi &) = delete;

	void build_surface(glfw_window &window, instance &instance);
	void build_swapchain(device &device);
	void acquire_image(semaphore &semaphore, u32 *image_index);

	struct
	{
		VkSurfaceKHR handle = {};
		instance *m_instance = {};
		glfw_window *m_window = {};
	} m_surface{};

	struct
	{
		VkSwapchainKHR m_handle = {};

		device *m_device = nullptr;

		VkFormat m_format = {};
		VkExtent2D m_extent = {};
		std::vector<VkImage> m_vulkan_images = {};
		std::vector<ref<image>> m_images = {};
		std::vector<ref<image_view>> m_image_views = {};
	} m_swapchain = {};

private:
};

} /* namespace vulkan */
