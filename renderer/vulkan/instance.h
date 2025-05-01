#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <vulkan/vulkan_profiles.hpp>
#pragma clang diagnostic pop

#include <platform/window.h>

namespace vulkan
{

class instance
{
public:
	instance() = default;
	~instance() = default;

	instance(const instance &) = delete;
	instance operator=(const instance &) = delete;

	void add_extension(const char *extension);
	void build(glfw_window &window, const VpProfileProperties &vp_profile_properties);
	void destroy();

	VkInstance m_handle = {};

private:
	void check_extensions_available();

	std::vector<const char *> m_extensions = {};
};

} /* namespace vulkan */
