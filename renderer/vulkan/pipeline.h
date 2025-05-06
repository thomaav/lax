#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

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

	void add_resource_bindings(shader_resource_binding *bindings, u32 binding_count);
	void build(device &device);

	VkPipelineLayout m_handle = {};

private:
	VkDevice m_device_handle = {};
	std::vector<shader_resource_binding> m_resource_bindings = {};
	descriptor_set_layout m_dset_layout = {};
};

class pipeline
{
public:
	pipeline() = default;
	~pipeline();

	pipeline(const pipeline &) = delete;
	pipeline operator=(const pipeline &) = delete;

	void add_shader(shader_module &shader);
	void build(device &device, render_pass &render_pass, VkExtent2D extent);

	VkPipeline m_handle = {};

	/* (TODO, thoave01): Hide again? */
	pipeline_layout m_pipeline_layout = {};

private:
	VkDevice m_device_handle = {};
	std::unordered_map<VkShaderStageFlagBits, shader_module *> m_shader_modules = {};
};

} /* namespace vulkan */
