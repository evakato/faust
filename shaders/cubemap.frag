#version 450

layout(location = 0) in vec3 inUVW;
layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform samplerCube skybox;

void main() {
    outColor = texture(skybox, inUVW);
}