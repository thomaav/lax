#pragma once

#include <optional>
#include <vulkan/vulkan.h>

#include <renderer/vulkan/instance.h>
#include <utils/type.h>

namespace Vulkan
{

class Device
{
public:
	Device() = default;
	~Device() = default;

	Device(const Device &) = delete;
	Device operator=(const Device &) = delete;

	struct
	{
		VkPhysicalDevice handle{};

		struct
		{
			/* (TODO, thoave01): Populate non-all ones? */
			std::optional<u32> graphics{};

			std::optional<u32> compute{};

			std::optional<u32> transfer{};

			std::optional<u32> all{};
		} queueFamily{};
	} physical{};

	struct
	{
		VkDevice handle{};
	} logical{};

	void addExtension(const char *extension);

	void logInfo();

	void build(Instance &instance, VkSurfaceKHR surface);

	void destroy();

	void wait();

private:
	std::vector<const char *> extensions{};

	void findPhysicalDevice(Instance &instance, VkSurfaceKHR surface);

	void createLogicalDevice();
};

} /* namespace Vulkan */
