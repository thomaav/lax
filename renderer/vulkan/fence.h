#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{

class Fence
{
public:
	Fence() = default;
	~Fence() = default;

	Fence(const Fence &) = delete;
	Fence operator=(const Fence &) = delete;

	VkFence handle{};

private:
};

} /* namespace Vulkan */
