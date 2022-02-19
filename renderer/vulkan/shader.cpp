#include <fstream>
#include <sstream>

#include <renderer/vulkan/shader.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace Vulkan
{

void Shader::build(Device &device, VkShaderStageFlagBits stage, const char *filename)
{
	std::string shaderCode{};
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.is_open())
		{
			terminate("Could not open shader %s", filename);
		}

		std::stringstream fileData{};
		fileData << file.rdbuf();
		shaderCode = fileData.str();
	}

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(shaderCode.c_str());

	VULKAN_ASSERT_SUCCESS(vkCreateShaderModule(device.logical.handle, &createInfo, nullptr, &module));

	this->device = device.logical.handle;
	this->stage = stage;
}

void Shader::destroy()
{
	vkDestroyShaderModule(device, module, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::getPipelineShaderStageCreateInfo()
{
	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = stage;
	shaderStageInfo.module = module;
	shaderStageInfo.pName = "main";
	return shaderStageInfo;
}

} /* namespace Vulkan */
