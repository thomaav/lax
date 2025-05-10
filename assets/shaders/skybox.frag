#version 460

layout(location = 0) in vec3 direction;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform samplerCube skybox;

void main()
{
	out_color = texture(skybox, direction);
}
