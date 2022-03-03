#include <algorithm>
#include <optional>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace
{

std::optional<u32> findQueueFamilyWithAllCapabilities(VkPhysicalDevice physicalDevice)
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

bool physicalDeviceHasRequiredExtensions(VkPhysicalDevice physicalDevice, std::vector<const char *> requiredExtensions)
{
	u32 deviceExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount,
	                                     availableDeviceExtensions.data());

	bool available = true;
	for (auto &extension : requiredExtensions)
	{
		/* (TODO, thoave01): Look into C++20 ranges? */
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

} /* namespace */

namespace Vulkan
{

void Device::addExtension(const char *extension)
{
	extensions.push_back(extension);
}

void Device::logInfo()
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physical.handle, &deviceProperties);
	info("Picked physical device %s", deviceProperties.deviceName);
}

/* (TODO, thoave01): Take Vulkan::Surface or whatever when we have abstraction. */
void Device::build(Instance &instance, VkSurfaceKHR surface)
{
	findPhysicalDevice(instance, surface);
	VULKAN_ASSERT_NOT_NULL(physical.handle);

	createLogicalDevice();
	VULKAN_ASSERT_NOT_NULL(logical.handle);
}

void Device::destroy()
{
	vkDestroyDevice(logical.handle, nullptr);
}

void Device::wait()
{
	vkDeviceWaitIdle(logical.handle);
}

void Device::findPhysicalDevice(Instance &instance, VkSurfaceKHR surface)
{
	u32 physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance.handle, &physicalDeviceCount, nullptr);

	if (physicalDeviceCount == 0)
	{
		terminate("No Vulkan physical device found");
	}

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance.handle, &physicalDeviceCount, physicalDevices.data());

	for (u32 i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);

		bool suiteable = true;
		{
			/* Must be discrete GPU. */
			suiteable &= deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

			/* Must have a queue with all capabilities (for now... simplest). */
			std::optional<u32> queueFamilyIndex = findQueueFamilyWithAllCapabilities(physicalDevices[i]);
			suiteable &= queueFamilyIndex.has_value();

			/* Must have all required device extensions. */
			suiteable &= physicalDeviceHasRequiredExtensions(physicalDevices[i], extensions);

			/* Also check for presentation support. */
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], *queueFamilyIndex, surface, &presentSupport);
			suiteable &= presentSupport;

			if (suiteable)
			{
				physical.handle = physicalDevices[i];
				physical.queueFamily.all = *queueFamilyIndex;

				logInfo();
				return;
			}
		}
	}

	terminate("No suitable physical device found");
}

void Device::createLogicalDevice()
{
	u32 queueIndex = findQueueFamilyWithAllCapabilities(physical.handle).value();
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

		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		VULKAN_ASSERT_SUCCESS(vkCreateDevice(physical.handle, &createInfo, nullptr, &logical.handle));
	}
}

} /* namespace Vulkan */
