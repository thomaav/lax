#version 460

layout(set = 0, binding = 1) uniform sampler2D texture;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(1.0, 0.0, 0.0, 1.0);
}
