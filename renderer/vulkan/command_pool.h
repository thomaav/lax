#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class CommandPool
{
public:
	CommandPool() = default;
	~CommandPool();

	CommandPool(const CommandPool &) = delete;
	CommandPool operator=(const CommandPool &) = delete;

	void build(Device &device);

	VkCommandPool handle{};

private:
	VkDevice device{};
};

} /* namespace Vulkan */
