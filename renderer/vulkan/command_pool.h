#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class CommandPool
{
public:
	CommandPool() = default;
	~CommandPool() = default;

	CommandPool(const CommandPool &) = delete;
	CommandPool operator=(const CommandPool &) = delete;

	void build(Device &device);

	void destroy();

	VkCommandPool handle{};

private:
	VkDevice device{};
};

} /* namespace Vulkan */
