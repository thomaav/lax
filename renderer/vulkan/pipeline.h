#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <utils/util.h>

#include "device.h"
#include "render_pass.h"
#include "shader.h"

namespace vulkan
{

class pipeline_layout
{
public:
	pipeline_layout() = default;
	~pipeline_layout();

	pipeline_layout(const pipeline_layout &) = delete;
	pipeline_layout operator=(const pipeline_layout &) = delete;

	void add_shader(const shader_module &shader);
	void build(device &device);

	VkPipelineLayout m_handle = {};
	u32 m_push_constants_size = 0;

private:
	VkDevice m_device_handle = {};
	std::vector<shader_resource_binding> m_resource_bindings = {};
	descriptor_set_layout m_dset_layout = {};
};

/* (TODO, thoave01): public no_copy_no_move inheritance. */

class pipeline
{
public:
	pipeline() = default;
	~pipeline();

	pipeline(const pipeline &) = delete;
	pipeline operator=(const pipeline &) = delete;

	void add_shader(device &device, VkShaderStageFlagBits stage, const char *path);
	void add_shader(const ref<shader_module> &shader);
	void set_sample_count(VkSampleCountFlagBits sample_count);

	void build(device &device, const render_pass &render_pass);
	void update(const render_pass &render_pass);

	VkPipeline m_handle = {};
	pipeline_layout m_pipeline_layout = {};

private:
	void finalize(const render_pass &render_pass);

	VkDevice m_device_handle = {};
	std::unordered_map<VkShaderStageFlagBits, ref<shader_module>> m_shader_modules = {};

	/* Configurable. */
	VkSampleCountFlagBits m_sample_count = VK_SAMPLE_COUNT_1_BIT;

	/* Set, and then re-used on update. */
	VkPipelineVertexInputStateCreateInfo m_vertex_input_info = {};
	VkPipelineInputAssemblyStateCreateInfo m_input_assembly = {};
	VkPipelineViewportStateCreateInfo m_viewport_info = {};
	VkPipelineRasterizationStateCreateInfo m_rasterizer_info = {};
	VkPipelineMultisampleStateCreateInfo m_multisampling_info = {};
	VkPipelineDepthStencilStateCreateInfo m_depth_stencil_info = {};
	VkPipelineColorBlendAttachmentState m_blend_attachment_state = {};
	VkPipelineColorBlendStateCreateInfo m_blending_info = {};
	const VkDynamicState m_dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo m_dynamic_state_info = {};
	std::vector<VkPipelineShaderStageCreateInfo> m_stage_create_infos = {};
};

} /* namespace vulkan */
