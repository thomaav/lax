#pragma once

#include <map>
#include <vector>
#include <vulkan/vulkan.h>

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
	void add_vertex_binding(u32 binding, size_t stride);
	void add_vertex_attribute(u32 binding, u32 location, VkFormat format, u32 offset);
	void build(device &device, pipeline_layout &pipeline_layout, render_pass &render_pass, VkExtent2D extent);

	VkPipeline m_handle = {};

private:
	std::vector<VkPipelineShaderStageCreateInfo> m_stages = {};
	std::vector<VkVertexInputBindingDescription> m_vertex_bindings = {};
	std::vector<VkVertexInputAttributeDescription> m_vertex_attributes = {};

	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
