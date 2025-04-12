#pragma once

#include <vector>

#include <third_party/volk/volk.h>

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
	void build();
	void destroy();

	VkInstance m_handle = {};

private:
	void check_extensions_available();

	std::vector<const char *> m_extensions = {};
};

} /* namespace vulkan */
