#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan
{

class Instance
{
public:
	Instance() = default;
	~Instance() = default;

	Instance(const Instance &) = delete;
	Instance operator=(const Instance &) = delete;

	VkInstance handle{};

	void addExtension(const char *extension);

	void addLayer(const char *layer);

	void build();

	void destroy();

private:
	std::vector<const char *> extensions{};

	std::vector<const char *> layers{};

	void checkExtensionsAvailable();

	void checkLayersAvailable();
};

} /* namespace Vulkan */
