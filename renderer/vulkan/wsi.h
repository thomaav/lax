#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include <platform/window.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/image.h>
#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/semaphore.h>

namespace vulkan
{

class wsi
{
public:
	wsi() = default;
	~wsi() = default;

	wsi(const wsi &) = delete;
	wsi operator=(const wsi &) = delete;

	struct
	{
		VkSurfaceKHR handle = {};
		instance *m_instance = {};
	} m_surface{};

	struct
	{
		VkSwapchainKHR m_handle = {};

		device *m_device = nullptr;

		VkFormat m_format = {};
		VkExtent2D m_extent = {};
		std::vector<VkImage> m_images = {};
		std::vector<std::unique_ptr<image>> m_vulkan_images = {};
		std::vector<std::unique_ptr<image_view>> m_image_views = {};
	} m_swapchain = {};

	struct
	{
		glfw_window m_handle = {};
		u32 m_width = 800;
		u32 m_height = 600;
	} window = {};

	void build_surface(instance &instance);
	void destroy_surface();
	void build_swapchain(device &device);
	void destroy_swapchain();
	void acquire_image(semaphore &semaphore, u32 *image_index);

private:
};

} /* namespace vulkan */
