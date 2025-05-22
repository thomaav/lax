#include <algorithm>
#include <optional>
#include <string_view>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/util.h>
#include <utils/log.h>
#include <utils/type.h>
#include <utils/util.h>

static std::optional<u32> find_queue_family_with_all_capabilities(VkPhysicalDevice physical_device)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

	for (u32 i = 0; i < queue_family_count; ++i)
	{
		VkQueueFamilyProperties &queue_family_properties = queue_families[i];
		if (queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
		    queue_family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT &&
		    queue_family_properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			return i;
		}
	}

	return {};
}

static bool physical_device_has_required_extensions(VkPhysicalDevice physical_device,
                                                    std::vector<const char *> required_extensions)
{
	u32 device_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);

	std::vector<VkExtensionProperties> available_device_extensions(device_extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count,
	                                     available_device_extensions.data());

	bool available = true;
	for (auto &extension : required_extensions)
	{
		auto v = std::find_if(
		    available_device_extensions.begin(), available_device_extensions.end(),
		    [&extension](const auto &availableDeviceExtension)
		    { return std::string_view{ extension } == std::string_view{ availableDeviceExtension.extensionName }; });

		if (v == available_device_extensions.end())
		{
			available = false;
		}
	}
	return available;
}

namespace vulkan
{

device::~device()
{
	if (VK_NULL_HANDLE != m_logical.m_handle)
	{
		vkDestroyDevice(m_logical.m_handle, nullptr);
	}
}

void device::add_extension(const char *extension)
{
	m_extensions.push_back(extension);
}

void device::log_info()
{
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(m_physical.m_handle, &device_properties);
	logger::info("Picked physical device %s", device_properties.deviceName);
}

void device::build(instance &instance, VkSurfaceKHR surface, const VpProfileProperties &vp_profile_properties)
{
	find_physical_device(instance, surface, false);
	VULKAN_ASSERT_NOT_NULL(m_physical.m_handle);

	create_logical_device(instance, vp_profile_properties);
	VULKAN_ASSERT_NOT_NULL(m_logical.m_handle);

	volkLoadDevice(m_logical.m_handle);
}

void device::wait()
{
	vkDeviceWaitIdle(m_logical.m_handle);
}

void device::find_physical_device(instance &instance, VkSurfaceKHR surface, bool must_be_discrete)
{
	u32 physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance.m_handle, &physical_device_count, nullptr);
	assert_if(physical_device_count == 0, "No Vulkan physical device found");

	std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
	vkEnumeratePhysicalDevices(instance.m_handle, &physical_device_count, physical_devices.data());

	for (u32 i = 0; i < physical_device_count; ++i)
	{
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;

		vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);
		vkGetPhysicalDeviceFeatures(physical_devices[i], &device_features);

		bool suitable = true;
		{
			if (must_be_discrete)
			{
				suitable &= device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			}

			/* Must have a queue with all capabilities (for now... simplest). */
			std::optional<u32> queue_family_index = find_queue_family_with_all_capabilities(physical_devices[i]);
			suitable &= queue_family_index.has_value();

			/* Must have all required device extensions. */
			suitable &= physical_device_has_required_extensions(physical_devices[i], m_extensions);

			/* Also check for presentation support. */
			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], *queue_family_index, surface, &present_support);
			suitable &= present_support;

			if (suitable)
			{
				m_physical.m_handle = physical_devices[i];
				m_physical.m_queue_family.m_all = *queue_family_index;

				log_info();
				return;
			}
		}
	}

	assert_if(true, "No suitable physical device found");
}

void device::create_logical_device(instance &instance, const VpProfileProperties &vp_profile_properties)
{
	VkBool32 profile_supported = true;
	vpGetPhysicalDeviceProfileSupport(instance.m_handle, m_physical.m_handle, &vp_profile_properties,
	                                  &profile_supported);
	assert_if(!profile_supported, "Requested Vulkan profile not supported, error at device creation");

	u32 queue_index = find_queue_family_with_all_capabilities(m_physical.m_handle).value();
	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = queue_index;
	queue_create_info.queueCount = 1;
	float queue_priority = 1.0f;
	queue_create_info.pQueuePriorities = &queue_priority;

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.queueCreateInfoCount = 1;
	add_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	device_create_info.enabledExtensionCount = m_extensions.size();
	device_create_info.ppEnabledExtensionNames = m_extensions.data();

	VpDeviceCreateInfo vp_device_create_info{};
	vp_device_create_info.pCreateInfo = &device_create_info;
	vp_device_create_info.pEnabledFullProfiles = &vp_profile_properties;
	vp_device_create_info.enabledFullProfileCount = 1;

	VULKAN_ASSERT_SUCCESS(vpCreateDevice(m_physical.m_handle, &vp_device_create_info, nullptr, &m_logical.m_handle));
}

} /* namespace vulkan */
