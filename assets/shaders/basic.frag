#version 460

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 out_color;

layout(std140, set = 0, binding = 0) uniform uniforms_block
{
	mat4 view;
	mat4 projection;
	uint enable_mipmapping;
} scene_uniforms;
layout(set = 0, binding = 1) uniform sampler2D diffuse;

layout(push_constant) uniform push_constants_block
{
	mat4 model;
} object_uniforms;

void main()
{
	if (scene_uniforms.enable_mipmapping == 0u)
	{
		out_color = textureLod(diffuse, uv, 0);
	}
	else
	{
		out_color = texture(diffuse, uv);
	}
}
