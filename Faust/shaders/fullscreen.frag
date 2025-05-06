#version 450

layout(set = 0, binding = 0) uniform sampler2D filteredResult;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(filteredResult, uv);
    outColor = color;
}
