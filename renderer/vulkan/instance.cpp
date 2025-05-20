#include <algorithm>
#include <string_view>

#include <renderer/log.h>
#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                     void *user_data)
{
	UNUSED(severity);
	UNUSED(type);
	UNUSED(user_data);

	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		logger::warn(callback_data->pMessage);
	}

	return VK_FALSE;
}

namespace vulkan
{

instance::~instance()
{
	if (VK_NULL_HANDLE != m_debug_messenger)
	{
		vkDestroyDebugUtilsMessengerEXT(m_handle, m_debug_messenger, nullptr);
	}
	if (VK_NULL_HANDLE != m_handle)
	{
		vkDestroyInstance(m_handle, nullptr);
	}
}

void instance::add_extension(const char *extension)
{
	m_extensions.push_back(extension);
}

void instance::check_layers_available()
{
	u32 layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	bool validation_layer_found = false;
	for (const auto &layer : available_layers)
	{
		if (strcmp("VK_LAYER_KHRONOS_validation", layer.layerName) == 0)
		{
			validation_layer_found = true;
			break;
		}
	}

	if (!validation_layer_found)
	{
		logger::info("VK_LAYER_KHRONOS_validation not available, continuing without");
		return;
	}
	m_layers.push_back("VK_LAYER_KHRONOS_validation");
	m_validation_layer_enabled = true;
}

void instance::check_extensions_available()
{
	if (m_validation_layer_enabled)
	{
		m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

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

void instance::setup_debugging()
{
	if (m_validation_layer_enabled)
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debug_callback;
		create_info.pUserData = nullptr;
		vkCreateDebugUtilsMessengerEXT(m_handle, &create_info, nullptr, &m_debug_messenger);
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
	check_layers_available();
	instance_create_info.enabledExtensionCount = m_layers.size();
	instance_create_info.ppEnabledExtensionNames = m_layers.data();
	check_extensions_available();
	instance_create_info.enabledExtensionCount = m_extensions.size();
	instance_create_info.ppEnabledExtensionNames = m_extensions.data();

	VpInstanceCreateInfo vp_instance_create_info = {};
	vp_instance_create_info.pEnabledFullProfiles = &vp_profile_properties;
	vp_instance_create_info.enabledFullProfileCount = 1;
	vp_instance_create_info.pCreateInfo = &instance_create_info;

	VULKAN_ASSERT_SUCCESS(vpCreateInstance(&vp_instance_create_info, nullptr, &m_handle));
	volkLoadInstance(m_handle);
	setup_debugging();
}

} /* namespace vulkan */
