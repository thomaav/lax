#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include <platform/window.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/queue.h>
#include <renderer/vulkan/wsi.h>
#include <utils/type.h>

namespace Vulkan
{

class Context
{
public:
	Context() = default;
	~Context();

	Context(const Context &) = delete;
	Context operator=(const Context &) = delete;

	void addInstanceExtension(const char *extension);

	void addInstanceLayer(const char *layer);

	void addDeviceExtension(const char *extension);

	/* (TODO, thoave01): Error handling? */
	void build();

private:
	Instance instance{};

	Device device{};

	WSI wsi{};

	Queue queue{};
};

} /* namespace Vulkan */
