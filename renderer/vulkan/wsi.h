#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include <platform/window.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/image.h>
#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/semaphore.h>

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

		/* (TODO, thoave01): Initializing this stuff as Vulkan::Image properly. */
		/* (TODO, thoave01): Requires some fixing of the swapchain in the WSI. */
		/* (TODO, thoave01): Maybe make the swapchain class at the same time. */
		std::vector<VkImage> images{};

		std::vector<std::unique_ptr<Image>> vulkanImages{};

		std::vector<std::unique_ptr<ImageView>> imageViews{};
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

	void acquireImage(Semaphore &semaphore, u32 *imageIndex);

private:
};

} /* namespace Vulkan */
