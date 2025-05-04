#include <fstream>
#include <sstream>

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <renderer/vulkan/shader.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace vulkan
{

shader_module::~shader_module()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkDestroyShaderModule(m_device->m_logical.m_handle, m_handle, nullptr);
	}
}

static VkFormat spirv_type_to_vkformat(const spirv_cross ::SPIRType &type)
{
	if (type.basetype == spirv_cross::SPIRType::Float && type.columns == 1)
	{
		switch (type.vecsize)
		{
		case 1:
			return VK_FORMAT_R32_SFLOAT;
		case 2:
			return VK_FORMAT_R32G32_SFLOAT;
		case 3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case 4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
	}

	terminate("spirv_type_to_vkformat found unsupported spirv_cross::SPIRType");
	return VK_FORMAT_UNDEFINED;
}

void shader_module::reflect(std::vector<u32> &binary)
{
	spirv_cross::Compiler compiler(binary);
	const auto &resources = compiler.get_shader_resources();

	/* Attributes. */
	if (m_stage == VK_SHADER_STAGE_VERTEX_BIT)
	{
		constexpr u32 default_binding = 0;
		u32 stride = 0;
		for (const auto &input : resources.stage_inputs)
		{
			const spirv_cross::SPIRType &type = compiler.get_type(input.type_id);
			VkVertexInputAttributeDescription attr = {};
			attr.location = compiler.get_decoration(input.id, spv::DecorationLocation);
			attr.binding = default_binding;
			attr.format = spirv_type_to_vkformat(type);
			attr.offset = stride;
			m_vads.push_back(attr);
			stride += type.vecsize * sizeof(float);

			info("location %u name %s vecsize %u", attr.location, input.name.c_str(), type.vecsize);
		}

		m_vbd.binding = default_binding;
		m_vbd.stride = stride;
		m_vbd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	/* Descriptor sets. */
	for (const auto &image : resources.sampled_images)
	{
		UNUSED(image);
		terminate("No support for sampled image descriptors");
	}
	for (const auto &image : resources.subpass_inputs)
	{
		UNUSED(image);
		terminate("No support for subpass input image descriptors");
	}
	for (const auto &image : resources.separate_images)
	{
		UNUSED(image);
		terminate("No support for separate image descriptors");
	}
	for (const auto &sampler : resources.separate_samplers)
	{
		UNUSED(sampler);
		terminate("No support for separate sampler descriptors");
	}
	for (const auto &image : resources.storage_images)
	{
		UNUSED(image);
		terminate("No support for storage image descriptors");
	}
	for (const auto &buffer : resources.uniform_buffers)
	{
		if (compiler.get_decoration(buffer.id, spv::DecorationDescriptorSet) != 0)
		{
			terminate("Only single descriptor set at index 0 supported");
		}

		const u32 binding = compiler.get_decoration(buffer.id, spv::DecorationBinding);
		m_dsl.add_binding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

		info("uniform buffer set %u binding %u name %s",
		     compiler.get_decoration(buffer.id, spv::DecorationDescriptorSet), binding, buffer.name.c_str());
	}
	for (const auto &buffer : resources.storage_buffers)
	{
		UNUSED(buffer);
		terminate("No support for storage buffer descriptors");
	}
	m_dsl.build(*m_device);

	/* Push constants. */
	if (!resources.push_constant_buffers.empty())
	{
		terminate("No support for push constants");
	}

	/* Specialization constants. */
	auto spec_constants = compiler.get_specialization_constants();
	for (const auto &constants : spec_constants)
	{
		UNUSED(constants);
		terminate("No support for specialization constants");
	}
}

void shader_module::build(device &device, VkShaderStageFlagBits stage, const char *filename)
{
	m_device = &device;

	m_stage = stage;
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

	/* Get attributes, descriptors, push constants. */
	const u32 *shader_code_ptr = reinterpret_cast<const u32 *>(shader_code.data());
	std::vector<u32> shader_code_vector(shader_code_ptr, shader_code_ptr + shader_code.size() / sizeof(u32));
	reflect(shader_code_vector);

	/* Finalize Vulkan module. */
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_code.size();
	create_info.pCode = reinterpret_cast<const uint32_t *>(shader_code.c_str());

	VULKAN_ASSERT_SUCCESS(vkCreateShaderModule(m_device->m_logical.m_handle, &create_info, nullptr, &m_handle));
}

VkPipelineShaderStageCreateInfo shader_module::get_pipeline_shader_stage_create_info() const
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
