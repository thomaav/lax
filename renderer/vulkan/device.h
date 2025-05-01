#pragma once

#include <optional>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/instance.h>
#include <utils/type.h>

namespace vulkan
{

class device
{
public:
	device() = default;
	~device() = default;

	device(const device &) = delete;
	device operator=(const device &) = delete;

	struct
	{
		VkPhysicalDevice m_handle = {};
		struct
		{
			std::optional<u32> m_graphics = {}; /* unused */
			std::optional<u32> m_compute = {};  /* unused */
			std::optional<u32> m_transfer = {}; /* unused */
			std::optional<u32> m_all = {};
		} m_queue_family = {};
	} m_physical = {};

	struct
	{
		VkDevice m_handle = {};
	} m_logical = {};

	void add_extension(const char *extension);
	void log_info();
	void build(instance &instance, VkSurfaceKHR surface, const VpProfileProperties &vp_profile_properties);
	void destroy();
	void wait();

private:
	void find_physical_device(instance &instance, VkSurfaceKHR surface, bool must_be_discrete);
	void create_logical_device(instance &instance, const VpProfileProperties &vp_profile_properties);

	std::vector<const char *> m_extensions = {};
};

} /* namespace vulkan */
