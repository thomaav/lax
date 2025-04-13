#pragma once

#include <vector>

#include <third_party/volk/volk.h>
#include <vulkan/vulkan_profiles.hpp>

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
