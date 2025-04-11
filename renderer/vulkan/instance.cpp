#include <algorithm>
#include <string_view>

#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace vulkan
{

void instance::add_extension(const char *extension)
{
	m_extensions.push_back(extension);
}

void instance::check_extensions_available()
{
	u32 instance_extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);

	std::vector<VkExtensionProperties> available_instance_extensions(instance_extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, available_instance_extensions.data());

	for (auto &extension : m_extensions)
	{
		auto v = std::find_if(
		    available_instance_extensions.begin(), available_instance_extensions.end(),
		    [&extension](const auto &available_extension)
		    { return std::string_view{ extension } == std::string_view{ available_extension.extensionName }; });

		if (v == available_instance_extensions.end())
		{
			terminate("Extension %s not found", extension);
		}
	}
}

void instance::build()
{
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = nullptr;
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = nullptr;
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;

	check_extensions_available();
	instance_info.enabledExtensionCount = m_extensions.size();
	instance_info.ppEnabledExtensionNames = m_extensions.data();

	instance_info.enabledLayerCount = 0;
	instance_info.ppEnabledLayerNames = nullptr;

	VULKAN_ASSERT_SUCCESS(vkCreateInstance(&instance_info, nullptr, &m_handle));
}

void instance::destroy()
{
	vkDestroyInstance(m_handle, nullptr);
}

} /* namespace vulkan */
