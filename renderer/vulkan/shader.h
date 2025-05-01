#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

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

	VkShaderModule m_handle = {};

private:
	VkDevice m_device_handle = {};
	VkShaderStageFlagBits m_stage = {};
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
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
