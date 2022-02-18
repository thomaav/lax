#pragma once

#include <vector>

#include <platform/window.h>
#include <utils/type.h>
#include <vulkan/vulkan.h>

namespace Vulkan
{

class Context
{
public:
	Context();
	~Context();

	Context(const Context &) = delete;
	Context operator=(const Context &) = delete;

	void addInstanceExtension(const char *extension);

	void addInstanceLayer(const char *layer);

	void addDeviceExtension(const char *extension);

	/* (TODO, thoave01): Error handling. */
	void build();

private:
	VkInstance instance{};

	/* (TODO, thoave01): Separate concepts. */
	VkDevice device{};

	/* (TODO, thoave01): Split presentation and graphics queue? */
	VkQueue queue{};

	/* (TODO, thoave01): Group surface and window elsewhere. */
	VkSurfaceKHR surface{};

	glfwWindow window{};

	std::vector<const char *> instanceExtensions{};

	std::vector<const char *> instanceLayers{};

	std::vector<const char *> deviceExtensions{};

	void checkInstanceExtensionsAvailable();

	void checkInstanceLayersAvailable();

	void initInstance();

	VkPhysicalDevice findFirstSuitablePhysicalDevice(u32 physicalDeviceCount, VkPhysicalDevice *physicalDevices);

	bool deviceExtensionsAvailable(VkPhysicalDevice physicalDevice);

	void initDevice();
};

}
