#include <fstream>
#include <sstream>

#include <renderer/vulkan/shader.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace vulkan
{

void shader::build(device &device, VkShaderStageFlagBits stage, const char *filename)
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

	VULKAN_ASSERT_SUCCESS(vkCreateShaderModule(device.m_logical.m_handle, &create_info, nullptr, &m_module));

	m_device_handle = device.m_logical.m_handle;
	m_stage = stage;
}

void shader::destroy()
{
	vkDestroyShaderModule(m_device_handle, m_module, nullptr);
}

VkPipelineShaderStageCreateInfo shader::get_pipeline_shader_stage_create_info()
{
	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = m_stage;
	shaderStageInfo.module = m_module;
	shaderStageInfo.pName = "main";
	return shaderStageInfo;
}

} /* namespace vulkan */
