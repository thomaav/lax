#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{

class Context
{
public:
	Context();
	~Context();

	Context(const Context &) = delete;
	Context operator=(const Context &) = delete;

	/* (TODO, thoave01): Error handling. */
	void init();

private:
	VkInstance instance;

	void initInstance();
};

}
