#include <renderer/vulkan/pipeline.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace vulkan
{

pipeline_layout::~pipeline_layout()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_device_handle, m_handle, nullptr);
	}
}

void pipeline_layout::add_resource_bindings(shader_resource_binding *bindings, u32 binding_count)
{
	for (u32 i = 0; i < binding_count; ++i)
	{
		if (std::any_of(m_resource_bindings.begin(), m_resource_bindings.end(),
		                [bindings, i](const shader_resource_binding &b) { return b.binding == bindings[i].binding; }))
		{
			/* Don't allow duplicate bindings. */
			continue;
		}
		m_resource_bindings.push_back(bindings[i]);
	}
}

void pipeline_layout::build(device &device)
{
	for (const shader_resource_binding &binding : m_resource_bindings)
	{
		if (binding.set != 0)
		{
			terminate("Only descriptor set index 0 is supported");
		}
		m_dset_layout.add_binding(binding.binding, binding.type);
	}
	m_dset_layout.build(device);

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &m_dset_layout.m_handle;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	VULKAN_ASSERT_SUCCESS(vkCreatePipelineLayout(device.m_logical.m_handle, &pipeline_layout_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
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
	terminate_if(m_shader_modules.contains(stage), "Pipeline already contains stage %u", stage);
	ref<shader_module> sm = make_ref<shader_module>();
	sm->build(device, stage, path);
	m_shader_modules[stage] = sm;
}

void pipeline::add_shader(const ref<shader_module> &shader)
{
	terminate_if(shader->m_handle == VK_NULL_HANDLE, "Shader has not been built");
	terminate_if(m_shader_modules.contains(shader->m_stage), "Pipeline already contains stage %u", shader->m_stage);
	m_shader_modules[shader->m_stage] = shader;
}

void pipeline::build(device &device, const render_pass &render_pass)
{
	if (!m_shader_modules.contains(VK_SHADER_STAGE_VERTEX_BIT))
	{
		terminate("Cannot build a pipeline without a vertex shader");
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	ref<shader_module> vertex_shader = m_shader_modules[VK_SHADER_STAGE_VERTEX_BIT];
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &vertex_shader->m_vbd;
	vertex_input_info.vertexAttributeDescriptionCount = vertex_shader->m_vads.size();
	vertex_input_info.pVertexAttributeDescriptions = vertex_shader->m_vads.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.pViewports = nullptr;
	viewport_info.scissorCount = 1;
	viewport_info.pScissors = nullptr;

	VkPipelineRasterizationStateCreateInfo rasterizer_info = {};
	rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_info.depthClampEnable = VK_FALSE;
	rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer_info.depthBiasEnable = VK_FALSE;
	rasterizer_info.depthBiasConstantFactor = 0.0f;
	rasterizer_info.depthBiasClamp = 0.0f;
	rasterizer_info.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling_info = {};
	multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling_info.sampleShadingEnable = VK_FALSE;
	multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling_info.minSampleShading = 1.0f;
	multisampling_info.pSampleMask = nullptr;
	multisampling_info.alphaToCoverageEnable = VK_FALSE;
	multisampling_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, //
		.pNext = nullptr,                                                    //
		.flags = 0,                                                          //
		.depthTestEnable = VK_TRUE,                                          //
		.depthWriteEnable = VK_TRUE,                                         //
		/* (TODO, thoave01): This is just for the skybox. */                 //
		.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,                       //
		.depthBoundsTestEnable = VK_FALSE,                                   //
		.stencilTestEnable = VK_FALSE,                                       //
		.front = {},                                                         //
		.back = {},                                                          //
		.minDepthBounds = 0.0f,                                              //
		.maxDepthBounds = 0.0f,                                              //
	};

	VkPipelineColorBlendAttachmentState blend_attachment_state = {};
	blend_attachment_state.colorWriteMask =
	    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blend_attachment_state.blendEnable = VK_FALSE;
	blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo blending_info = {};
	blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blending_info.logicOpEnable = VK_FALSE;
	blending_info.logicOp = VK_LOGIC_OP_COPY;
	blending_info.attachmentCount = 1;
	blending_info.pAttachments = &blend_attachment_state;
	blending_info.blendConstants[0] = 0.0f;
	blending_info.blendConstants[1] = 0.0f;
	blending_info.blendConstants[2] = 0.0f;
	blending_info.blendConstants[3] = 0.0f;

	const VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, //
		.pNext = nullptr,                                              //
		.flags = 0,                                                    //
		.dynamicStateCount = 2,                                        //
		.pDynamicStates = dynamic_states,                              //
	};

	std::vector<VkPipelineShaderStageCreateInfo> stage_create_infos = {};
	for (const auto &sm : m_shader_modules)
	{
		stage_create_infos.push_back(sm.second->get_pipeline_shader_stage_create_info());
	}
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = stage_create_infos.size();
	pipeline_info.pStages = stage_create_infos.data();

	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &rasterizer_info;
	pipeline_info.pMultisampleState = &multisampling_info;
	pipeline_info.pDepthStencilState = &depth_stencil_info;
	pipeline_info.pColorBlendState = &blending_info;
	pipeline_info.pDynamicState = &dynamic_state_info;

	for (const auto &sm : m_shader_modules)
	{
		ref<shader_module> sm_ptr = sm.second;
		m_pipeline_layout.add_resource_bindings(sm_ptr->m_resource_bindings.data(), sm_ptr->m_resource_bindings.size());
	}
	m_pipeline_layout.build(device);
	pipeline_info.layout = m_pipeline_layout.m_handle;

	pipeline_info.renderPass = render_pass.m_handle;
	pipeline_info.subpass = 0;
	/* (TODO, thoave01): More general way of handling pNexts. */
	pipeline_info.pNext = VK_NULL_HANDLE == render_pass.m_handle ? &render_pass.m_rendering_info : nullptr;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	vkCreateGraphicsPipelines(device.m_logical.m_handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_handle);

	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
