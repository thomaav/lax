#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/command_pool.h>
#include <renderer/vulkan/device.h>

namespace Vulkan
{

class CommandBuffer
{
public:
	CommandBuffer() = default;
	~CommandBuffer() = default;

	CommandBuffer(const CommandBuffer &) = delete;
	CommandBuffer operator=(const CommandBuffer &) = delete;

	void build(Device &device, CommandPool &commandPool);

	void begin();

	void end();

	VkCommandBuffer handle{};

private:
	VkDevice device{};

	VkCommandPool commandPool{};
};

} /* namespace Vulkan */
