#include <renderer/vulkan/pipeline.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

pipeline_layout::~pipeline_layout()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_device_handle, m_handle, nullptr);
	}
}

void pipeline_layout::build(device &device)
{
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = nullptr;
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

void pipeline::add_shader(shader_module &shader)
{
	assert(shader.m_handle != VK_NULL_HANDLE);
	m_stages.push_back(shader.get_pipeline_shader_stage_create_info());
}

void pipeline::build(device &device, pipeline_layout &pipeline_layout, render_pass &render_pass, VkExtent2D extent)
{
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = nullptr;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.pViewports = &viewport;
	viewport_info.scissorCount = 1;
	viewport_info.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer_info = {};
	rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_info.depthClampEnable = VK_FALSE;
	rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = m_stages.data();

	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &rasterizer_info;
	pipeline_info.pMultisampleState = &multisampling_info;
	pipeline_info.pDepthStencilState = nullptr;
	pipeline_info.pColorBlendState = &blending_info;
	pipeline_info.pDynamicState = nullptr;

	pipeline_info.layout = pipeline_layout.m_handle;

	pipeline_info.renderPass = render_pass.m_handle;
	pipeline_info.subpass = 0;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	vkCreateGraphicsPipelines(device.m_logical.m_handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_handle);

	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
