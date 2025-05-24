#pragma once

#include <renderer/vulkan/context.h>

class ui
{
public:
	ui() = default;
	~ui();

	ui(const ui &) = delete;
	ui operator=(const ui &) = delete;

	void build(vulkan::context &context);

private:
};
