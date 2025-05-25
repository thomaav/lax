#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <platform/window.h>

class VpProfileProperties;

namespace vulkan
{

class instance
{
public:
	instance() = default;
	~instance();

	instance(const instance &) = delete;
	instance operator=(const instance &) = delete;

	void add_extension(const char *extension);
	void build(glfw_window &window, const VpProfileProperties &vp_profile_properties);

	VkInstance m_handle = {};

private:
	void check_layers_available();
	void check_extensions_available();
	void setup_debugging();

	std::vector<const char *> m_layers = {};
	std::vector<const char *> m_extensions = {};

	VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
	bool m_validation_layer_enabled = false;
};

} /* namespace vulkan */
