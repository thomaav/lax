#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv_in;
layout(location = 3) in vec4 color;

layout(std140, set = 0, binding = 0) uniform ubo_block
{
	mat4 view;
	mat4 projection;
	uint enable_mipmapping;
} scene_uniforms;

void main()
{
	gl_Position = scene_uniforms.projection * scene_uniforms.view * vec4(position, 1.0f);
}
