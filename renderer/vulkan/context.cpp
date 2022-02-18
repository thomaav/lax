#include <array>
#include <iostream>
#include <optional>
#include <string_view>

#include <platform/window.h>
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
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void Context::addInstanceExtension(const char *extension)
{
	instanceExtensions.push_back(extension);
}

void Context::checkInstanceExtensionsAvailable()
{
	u32 instanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableInstanceExtensions.data());

	for (auto &extension : instanceExtensions)
	{
		auto v = std::find_if(availableInstanceExtensions.begin(), availableInstanceExtensions.end(),
		                      [&extension](const auto &availableExtension) {
			                      return std::string_view{ extension } ==
			                             std::string_view{ availableExtension.extensionName };
		                      });

		if (v == availableInstanceExtensions.end())
		{
			terminate("Extension %s not found", extension);
		}
	}
}

void Context::addInstanceLayer(const char *layer)
{
	instanceLayers.push_back(layer);
}

void Context::addDeviceExtension(const char *extension)
{
	/* (TODO, thoave01): Assert if we already built the context. */
	deviceExtensions.push_back(extension);
}

void Context::checkInstanceLayersAvailable()
{
	u32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

	std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data());

	for (auto &layer : instanceLayers)
	{
		auto v =
		    std::find_if(availableInstanceLayers.begin(), availableInstanceLayers.end(),
		                 [&layer](const auto &availableInstanceLayer) {
			                 return std::string_view{ layer } == std::string_view{ availableInstanceLayer.layerName };
		                 });

		if (v == availableInstanceLayers.end())
		{
			terminate("Layer %s not found", layer);
		}
	}
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

	checkInstanceExtensionsAvailable();
	instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

	checkInstanceLayersAvailable();
	instanceCreateInfo.enabledLayerCount = instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();

	VULKAN_ASSERT_SUCCESS(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
}

/* (TODO, thoave01): Anonymous namespace? */
static std::optional<u32> findQueueFamilyWithAllCapabilities(VkPhysicalDevice physicalDevice)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	for (u32 i = 0; i < queueFamilyCount; ++i)
	{
		VkQueueFamilyProperties &queueFamilyProperties = queueFamilies[i];
		if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
		    queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT &&
		    queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			return i;
		}
	}

	return {};
}

VkPhysicalDevice Context::findFirstSuitablePhysicalDevice(u32 physicalDeviceCount, VkPhysicalDevice *physicalDevices)
{
	for (u32 i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);

		std::optional<u32> queueFamilyIndex = findQueueFamilyWithAllCapabilities(physicalDevices[i]);

		bool suiteable = true;
		{
			suiteable &= deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			/* (TODO, thoave01): Device features required? */
			suiteable &= queueFamilyIndex.has_value();
			suiteable &= deviceExtensionsAvailable(physicalDevices[i]);
		}

		if (suiteable)
		{
			/* Also check for presentation support. */
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], *queueFamilyIndex, surface, &presentSupport);

			if (presentSupport)
			{
				return physicalDevices[i];
			}
		}
	}

	terminate("No discrete physical device found");
	return VK_NULL_HANDLE;
}

static void logPhysicalDeviceInfo(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	info("Picked physical device %s", deviceProperties.deviceName);
}

bool Context::deviceExtensionsAvailable(VkPhysicalDevice physicalDevice)
{
	u32 deviceExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount,
	                                     availableDeviceExtensions.data());

	bool available = true;
	for (auto &extension : deviceExtensions)
	{
		auto v = std::find_if(availableDeviceExtensions.begin(), availableDeviceExtensions.end(),
		                      [&extension](const auto &availableDeviceExtension) {
			                      return std::string_view{ extension } ==
			                             std::string_view{ availableDeviceExtension.extensionName };
		                      });

		if (v == availableDeviceExtensions.end())
		{
			available = false;
		}
	}
	return available;
}

/* (TODO, thoave01): Decouple everything etc. etc. */
void Context::initDevice()
{
	VULKAN_ASSERT_NOT_NULL(instance);

	/* (TODO, thoave01): Surface code should be platform specific? */
	/* Create surface. */
	{
		window.init(800, 600);
		VULKAN_ASSERT_SUCCESS(window.createVulkanSurface(instance, &surface));
	}

	/* Create physical device. */
	VkPhysicalDevice physicalDevice{};
	{
		u32 physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

		if (physicalDeviceCount == 0)
		{
			terminate("No Vulkan physical device found");
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		physicalDevice = findFirstSuitablePhysicalDevice(physicalDeviceCount, physicalDevices.data());
		logPhysicalDeviceInfo(physicalDevice);
	}

	/* Create swapchain. */
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

		if (formats.empty() || presentModes.empty())

		{
			terminate("No suiteable swapchain formats");
		}

		if (presentModes.empty())

		{
			terminate("No suiteable swapchain present modes");
		}
	}

	/* Create logical device. */
	u32 queueIndex = findQueueFamilyWithAllCapabilities(physicalDevice).value();
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueIndex;
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		VULKAN_ASSERT_SUCCESS(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));
	}

	/* Create queue. */
	{
		vkGetDeviceQueue(device, queueIndex, 0, &queue);
	}
}

void Context::build()
{
	initInstance();
	initDevice();
}

}
