#include <iostream>
#include <vector>

#include <renderer/vulkan/context.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace Vulkan
{

Context::Context()
{
}

Context::~Context()
{
	vkDestroyInstance(instance, nullptr);
}

void Context::initInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = nullptr;
	appInfo.applicationVersion = VK_NULL_HANDLE;
	appInfo.pEngineName = nullptr;
	appInfo.engineVersion = VK_NULL_HANDLE;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	/* (TODO, thoave01): Proper handling of surface extensions. */
	std::vector<const char *> instanceExtensions{};
	instanceExtensions.push_back("VK_KHR_surface");
	instanceExtensions.push_back("VK_KHR_xcb_surface");

	instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

	/* (TODO, thoave01): Add validation layers. */
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.ppEnabledLayerNames = nullptr;

	VULKAN_ASSERT_SUCCESS(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
}

void Context::build()
{
	initInstance();
}

}
