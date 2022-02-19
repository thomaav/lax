#pragma once

#include <vulkan/vulkan.h>

#include <platform/window.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/instance.h>

namespace Vulkan
{

/* (TODO, thoave01): Create full/individual surface and swapchain objects with destroy methods? */
class WSI
{
public:
	WSI() = default;
	~WSI() = default;

	WSI(const WSI &) = delete;
	WSI operator=(const WSI &) = delete;

	struct
	{
		VkSurfaceKHR handle{};

		Instance *instance{};
	} surface{};

	struct
	{
		VkSwapchainKHR handle{};

		/* (TODO, thoave01): Maybe just store the VkHandle? */
		/* (TODO, thoave01): Do we need the rest? Nah probably not. */
		Device *device{};

		VkFormat format{};

		VkExtent2D extent{};

		std::vector<VkImage> images{};

		std::vector<VkImageView> imageViews{};
	} swapchain{};

	/* (TODO, thoave01): Should WSI be aware of GLFW? Whatever. */
	struct
	{
		glfwWindow handle{};

		u32 width{ 800 };

		u32 height{ 600 };
	} window{};

	void buildSurface(Instance &instance);

	void destroySurface();

	void buildSwapchain(Device &device);

	void destroySwapchain();

private:
};

} /* namespace Vulkan */
