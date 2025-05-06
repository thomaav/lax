#version 460

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 uv;

layout(set = 0, binding = 1) uniform sampler2D diffuse;

void main() {
    out_color = texture(diffuse, uv);
}
