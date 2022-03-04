#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/shader.h>

namespace Vulkan
{

class PipelineLayout
{
public:
	PipelineLayout() = default;
	~PipelineLayout();

	PipelineLayout(const PipelineLayout &) = delete;
	PipelineLayout operator=(const PipelineLayout &) = delete;

	VkPipelineLayout handle{};

	void build(Device &device);

private:
	VkDevice device{};
};

class Pipeline
{
public:
	Pipeline() = default;
	~Pipeline();

	Pipeline(const Pipeline &) = delete;
	Pipeline operator=(const Pipeline &) = delete;

	VkPipeline handle{};

	void addShader(Shader &shader);

	void build(Device &device, PipelineLayout &pipelineLayout, RenderPass &renderPass, VkExtent2D extent);

private:
	std::vector<VkPipelineShaderStageCreateInfo> stages{};

	VkDevice device{};
};

} /* namespace Vulkan */
