#include <fstream>
#include <sstream>

#include <renderer/vulkan/shader.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace vulkan
{

shader_module::~shader_module()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkDestroyShaderModule(m_device_handle, m_handle, nullptr);
	}
}

void shader_module::build(device &device, VkShaderStageFlagBits stage, const char *filename)
{
	std::string shader_code = {};
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.is_open())
		{
			terminate("Could not open shader %s", filename);
		}

		std::stringstream file_data = {};
		file_data << file.rdbuf();
		shader_code = file_data.str();
	}

	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_code.size();
	create_info.pCode = reinterpret_cast<const uint32_t *>(shader_code.c_str());

	VULKAN_ASSERT_SUCCESS(vkCreateShaderModule(device.m_logical.m_handle, &create_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
	m_stage = stage;
}

VkPipelineShaderStageCreateInfo shader_module::get_pipeline_shader_stage_create_info()
{
	VkPipelineShaderStageCreateInfo shader_stage_info{};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.stage = m_stage;
	shader_stage_info.module = m_handle;
	shader_stage_info.pName = "main";
	return shader_stage_info;
}

shader_object::~shader_object()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkDestroyShaderEXT(m_device_handle, m_handle, nullptr);
	}
}

static VkShaderStageFlags get_next_stage(VkShaderStageFlagBits stage)
{
	switch (stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
		       VK_SHADER_STAGE_FRAGMENT_BIT;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	case VK_SHADER_STAGE_GEOMETRY_BIT:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return 0;
	case VK_SHADER_STAGE_COMPUTE_BIT:
		return 0;
	default:
		assert(false);
	}
}

void shader_object::build(device &device, VkShaderStageFlagBits stage, const char *filename)
{
	/* Read SPIR-V from filename. */
	std::string shader_code = {};
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		terminate("Could not open shader %s", filename);
	}
	std::stringstream file_data = {};
	file_data << file.rdbuf();
	shader_code = file_data.str();

	/* Initialize Vulkan shader object. */
	VkShaderCreateInfoEXT info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT, //
		.pNext = nullptr,                                  //
		.flags = 0,                                        //
		.stage = stage,                                    //
		.nextStage = get_next_stage(stage),                //
		.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,         //
		.codeSize = shader_code.size(),                    //
		.pCode = shader_code.c_str(),                      //
		.pName = "main",                                   //
		.setLayoutCount = 0,                               //
		.pSetLayouts = nullptr,                            //
		.pushConstantRangeCount = 0,                       //
		.pPushConstantRanges = nullptr,                    //
		.pSpecializationInfo = nullptr,                    //
	};
	VULKAN_ASSERT_SUCCESS(vkCreateShadersEXT(device.m_logical.m_handle, 1, &info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
