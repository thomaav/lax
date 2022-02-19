#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class Shader
{
public:
	Shader() = default;
	~Shader() = default;

	Shader(const Shader &) = delete;
	Shader operator=(const Shader &) = delete;

	VkShaderModule module{};

	VkDevice device{};

	VkShaderStageFlagBits stage{};

	void build(Device &device, VkShaderStageFlagBits stage, const char *filename);

	void destroy();

	VkPipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo();

private:
};

} /* namespace Vulkan */
