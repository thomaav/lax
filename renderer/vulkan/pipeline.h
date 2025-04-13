#pragma once

#include <map>
#include <vector>

#include <third_party/volk/volk.h>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/shader.h>

namespace vulkan
{

class pipeline_layout
{
public:
	pipeline_layout() = default;
	~pipeline_layout();

	pipeline_layout(const pipeline_layout &) = delete;
	pipeline_layout operator=(const pipeline_layout &) = delete;

	void build(device &device);

	VkPipelineLayout m_handle = {};

private:
	VkDevice m_device_handle = {};
};

class pipeline
{
public:
	pipeline() = default;
	~pipeline();

	pipeline(const pipeline &) = delete;
	pipeline operator=(const pipeline &) = delete;

	void add_shader(shader_module &shader);
	void build(device &device, pipeline_layout &pipeline_layout, render_pass &render_pass, VkExtent2D extent);

	VkPipeline m_handle = {};

private:
	VkDevice m_device_handle = {};
	std::vector<VkPipelineShaderStageCreateInfo> m_stages = {};
};

} /* namespace vulkan */
