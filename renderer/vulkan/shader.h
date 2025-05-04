#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/descriptor_set.h>
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
	VkPipelineShaderStageCreateInfo get_pipeline_shader_stage_create_info() const;

	VkShaderModule m_handle = {};
	VkShaderStageFlagBits m_stage = {};

	VkVertexInputBindingDescription m_vbd = {};
	std::vector<VkVertexInputAttributeDescription> m_vads = {};
	descriptor_set_layout m_dsl = {}; /* (TODO, thoave01): Single descriptor set for now. */

private:
	void reflect(std::vector<u32> &binary);

	device *m_device = nullptr;
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
