#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace vulkan
{

class shader_module
{
public:
	shader_module() = default;
	~shader_module() = default;

	shader_module(const shader_module &) = delete;
	shader_module operator=(const shader_module &) = delete;

	void build(vulkan::device &device, VkShaderStageFlagBits stage, const char *filename);
	void destroy();
	VkPipelineShaderStageCreateInfo get_pipeline_shader_stage_create_info();

	VkShaderModule m_module = {};
	VkDevice m_device_handle = {};
	VkShaderStageFlagBits m_stage = {};

private:
};

} /* namespace vulkan */
