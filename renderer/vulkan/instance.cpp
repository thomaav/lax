#include <algorithm>
#include <string_view>

#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace vulkan
{

instance::~instance()
{
	vkDestroyInstance(m_handle, nullptr);
}

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
		assert_if(v == available_instance_extensions.end(), "Extension %s not found", extension);
	}
}

void instance::build(glfw_window &window, const VpProfileProperties &vp_profile_properties)
{
	VkBool32 profile_supported = true;
	vpGetInstanceProfileSupport(nullptr, &vp_profile_properties, &profile_supported);
	assert_if(!profile_supported, "Requested Vulkan profile not supported, error at instance creation");

	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	for (const char *extension : window.get_required_surface_extensions())
	{
		add_extension(extension);
	}
	check_extensions_available();
	instance_create_info.enabledExtensionCount = m_extensions.size();
	instance_create_info.ppEnabledExtensionNames = m_extensions.data();

	VpInstanceCreateInfo vp_instance_create_info = {};
	vp_instance_create_info.pEnabledFullProfiles = &vp_profile_properties;
	vp_instance_create_info.enabledFullProfileCount = 1;
	vp_instance_create_info.pCreateInfo = &instance_create_info;

	VULKAN_ASSERT_SUCCESS(vpCreateInstance(&vp_instance_create_info, nullptr, &m_handle));
}

} /* namespace vulkan */
