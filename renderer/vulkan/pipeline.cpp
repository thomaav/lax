#include <utils/util.h>

#include "pipeline.h"
#include "util.h"

namespace vulkan
{

pipeline_layout::~pipeline_layout()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_device_handle, m_handle, nullptr);
	}
}

void pipeline_layout::add_shader(const shader_module &shader)
{
	/* Resource layout. */
	for (const shader_resource_binding &shader_binding : shader.m_resource_bindings)
	{
		if (std::any_of(m_resource_bindings.begin(), m_resource_bindings.end(),
		                [shader_binding](const shader_resource_binding &b)
		                { return b.binding == shader_binding.binding; }))
		{
			/* Don't allow duplicate bindings. */
			continue;
		}
		m_resource_bindings.push_back(shader_binding);
	}

	/* Push constants. */
	m_push_constants_size = std::max(m_push_constants_size, shader.m_push_constants_size);
}

void pipeline_layout::build(device &device)
{
	for (const shader_resource_binding &binding : m_resource_bindings)
	{
		assert_if(binding.set != 0, "Only descriptor set index 0 is supported");
		m_dset_layout.add_binding(binding.binding, binding.type);
	}
	m_dset_layout.build(device);

	VkPushConstantRange push_constants = {};
	/* (TODO, thoave01): Fix stage flags. */
	push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constants.offset = 0;
	push_constants.size = m_push_constants_size;
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &m_dset_layout.m_handle;
	if (m_push_constants_size > 0)
	{
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constants;
	}
	else
	{
		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;
	}

	VULKAN_ASSERT_SUCCESS(vkCreatePipelineLayout(device.m_logical.m_handle, &pipeline_layout_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

pipeline::pipeline()
{
	m_rendering_info = {};
	m_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	m_rendering_info.colorAttachmentCount = 1;
	m_color_format = VK_FORMAT_B8G8R8A8_SRGB;
	m_rendering_info.pColorAttachmentFormats = &m_color_format;
	m_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	m_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

	m_input_assembly = {};
	m_input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_input_assembly.primitiveRestartEnable = VK_FALSE;

	m_viewport_info = {};
	m_viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewport_info.viewportCount = 1;
	m_viewport_info.pViewports = nullptr;
	m_viewport_info.scissorCount = 1;
	m_viewport_info.pScissors = nullptr;

	m_rasterizer_info = {};
	m_rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterizer_info.depthClampEnable = VK_FALSE;
	m_rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	m_rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	m_rasterizer_info.lineWidth = 1.0f;
	m_rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
	m_rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	m_rasterizer_info.depthBiasEnable = VK_FALSE;
	m_rasterizer_info.depthBiasConstantFactor = 0.0f;
	m_rasterizer_info.depthBiasClamp = 0.0f;
	m_rasterizer_info.depthBiasSlopeFactor = 0.0f;

	m_multisampling_info = {};
	m_multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_multisampling_info.sampleShadingEnable = VK_FALSE;
	m_multisampling_info.minSampleShading = 1.0f;
	m_multisampling_info.pSampleMask = nullptr;
	m_multisampling_info.alphaToCoverageEnable = VK_FALSE;
	m_multisampling_info.alphaToOneEnable = VK_FALSE;

	m_depth_stencil_info = {};
	m_depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_depth_stencil_info.pNext = nullptr;
	m_depth_stencil_info.flags = 0;
	m_depth_stencil_info.depthTestEnable = VK_TRUE;
	m_depth_stencil_info.depthWriteEnable = VK_TRUE;
	m_depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	m_depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	m_depth_stencil_info.stencilTestEnable = VK_FALSE;
	m_depth_stencil_info.front = {};
	m_depth_stencil_info.back = {};
	m_depth_stencil_info.minDepthBounds = 0.0f;
	m_depth_stencil_info.maxDepthBounds = 0.0f;

	m_blend_attachment_state = {};
	m_blend_attachment_state.colorWriteMask =
	    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_blend_attachment_state.blendEnable = VK_FALSE;
	m_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	m_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	m_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	m_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	m_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	m_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	m_blending_info = {};
	m_blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_blending_info.logicOpEnable = VK_FALSE;
	m_blending_info.logicOp = VK_LOGIC_OP_COPY;
	m_blending_info.attachmentCount = 1;
	m_blending_info.pAttachments = &m_blend_attachment_state;
	m_blending_info.blendConstants[0] = 0.0f;
	m_blending_info.blendConstants[1] = 0.0f;
	m_blending_info.blendConstants[2] = 0.0f;
	m_blending_info.blendConstants[3] = 0.0f;

	m_dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	m_dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);
	m_dynamic_state_info = {};
	m_dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	m_dynamic_state_info.pNext = nullptr;
	m_dynamic_state_info.flags = 0;
	m_dynamic_state_info.dynamicStateCount = m_dynamic_states.size();
	m_dynamic_state_info.pDynamicStates = m_dynamic_states.data();
}

pipeline::~pipeline()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device_handle, m_handle, nullptr);
	}
}

void pipeline::add_shader(device &device, VkShaderStageFlagBits stage, const char *path)
{
	assert_if(m_shader_modules.contains(stage), "Pipeline already contains stage %u", stage);
	ref<shader_module> sm = make_ref<shader_module>();
	sm->build(device, stage, path);
	m_shader_modules[stage] = sm;
	m_pipeline_layout.add_shader(*sm);
	m_stage_create_infos.push_back(sm->get_pipeline_shader_stage_create_info());
}

void pipeline::add_shader(const ref<shader_module> &shader)
{
	assert_if(shader->m_handle == VK_NULL_HANDLE, "Shader has not been built");
	assert_if(m_shader_modules.contains(shader->m_stage), "Pipeline already contains stage %u", shader->m_stage);
	m_shader_modules[shader->m_stage] = shader;
	m_pipeline_layout.add_shader(*shader);
	m_stage_create_infos.push_back(shader->get_pipeline_shader_stage_create_info());
}

void pipeline::set_sample_count(VkSampleCountFlagBits sample_count)
{
	m_multisampling_info.rasterizationSamples = sample_count;
}

void pipeline::set_topology(VkPrimitiveTopology topology)
{
	m_input_assembly.topology = topology;
}

void pipeline::set_cull_mode(VkCullModeFlags cull_mode)
{
	m_rasterizer_info.cullMode = cull_mode;
}

void pipeline::set_blend_enable(VkBool32 blend_enable)
{
	if (blend_enable)
	{
		m_blend_attachment_state.colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		m_blend_attachment_state.blendEnable = VK_FALSE;
		m_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		m_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		m_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		m_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		m_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		m_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	else
	{
		m_blend_attachment_state.colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		m_blend_attachment_state.blendEnable = VK_TRUE;
		m_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		m_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		m_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		m_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	}
}

void pipeline::set_color_format(VkFormat format)
{
	m_color_format = format;
	m_rendering_info.pColorAttachmentFormats = &m_color_format;
}

void pipeline::set_depth_format(VkFormat format)
{
	m_rendering_info.depthAttachmentFormat = format;
}

void pipeline::finalize()
{
	/* Build the final pipeline. */
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = &m_rendering_info;
	pipeline_info.stageCount = m_stage_create_infos.size();
	pipeline_info.pStages = m_stage_create_infos.data();
	pipeline_info.pVertexInputState = &m_vertex_input_info;
	pipeline_info.pInputAssemblyState = &m_input_assembly;
	pipeline_info.pViewportState = &m_viewport_info;
	pipeline_info.pRasterizationState = &m_rasterizer_info;
	pipeline_info.pMultisampleState = &m_multisampling_info;
	pipeline_info.pDepthStencilState = &m_depth_stencil_info;
	pipeline_info.pColorBlendState = &m_blending_info;
	pipeline_info.pDynamicState = &m_dynamic_state_info;
	pipeline_info.layout = m_pipeline_layout.m_handle;
	pipeline_info.renderPass = VK_NULL_HANDLE;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;
	vkCreateGraphicsPipelines(m_device_handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_handle);
}

void pipeline::build(device &device)
{
	assert_if(!m_shader_modules.contains(VK_SHADER_STAGE_VERTEX_BIT),
	          "Cannot build a pipeline without a vertex shader");

	m_device_handle = device.m_logical.m_handle;

	m_vertex_input_info = {};
	m_vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (m_shader_modules[VK_SHADER_STAGE_VERTEX_BIT]->m_vads.size() != 0)
	{
		m_vertex_input_info.vertexBindingDescriptionCount = 1;
		m_vertex_input_info.pVertexBindingDescriptions = &m_shader_modules[VK_SHADER_STAGE_VERTEX_BIT]->m_vbd;
		m_vertex_input_info.vertexAttributeDescriptionCount =
		    m_shader_modules[VK_SHADER_STAGE_VERTEX_BIT]->m_vads.size();
		m_vertex_input_info.pVertexAttributeDescriptions = m_shader_modules[VK_SHADER_STAGE_VERTEX_BIT]->m_vads.data();
	}

	m_pipeline_layout.build(device);
	finalize();
}

void pipeline::update()
{
	VULKAN_ASSERT_NOT_NULL(m_handle);
	VULKAN_ASSERT_NOT_NULL(m_device_handle);
	vkDestroyPipeline(m_device_handle, m_handle, nullptr);
	finalize();
}

} /* namespace vulkan */
