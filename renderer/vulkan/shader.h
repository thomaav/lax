#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace vulkan
{

class shader_module
{
public:
	shader_module() = default;
	~shader_module();

	shader_module(const shader_module &) = delete;
	shader_module operator=(const shader_module &) = delete;

	void build(device &device, VkShaderStageFlagBits stage, const char *filename);
	VkPipelineShaderStageCreateInfo get_pipeline_shader_stage_create_info();

	VkShaderModule m_module = {};
	VkDevice m_device_handle = {};
	VkShaderStageFlagBits m_stage = {};

private:
};

class shader_object
{
public:
	shader_object() = default;
	~shader_object();

	shader_object(const shader_object &) = delete;
	shader_object operator=(const shader_object &) = delete;

	void build(device &device, VkShaderStageFlagBits stage, const char *filename);

	VkShaderEXT m_handle;

private:
};

} /* namespace vulkan */
