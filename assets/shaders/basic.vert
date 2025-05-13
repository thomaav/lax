#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv_in;
layout(location = 3) in vec4 color;

layout(location = 0) out vec2 uv_out;

layout(std140, set = 0, binding = 0) uniform uniforms_block
{
	mat4 model;
	mat4 view;
	mat4 projection;
	uint use_mipmap;
} uniforms;

void main() {
	uv_out = uv_in;
    gl_Position =
		uniforms.projection * uniforms.view * uniforms.model * vec4(position, 1.0f);
}
