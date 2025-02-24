#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform samplerCube skybox;
layout(location = 0) in vec3 texCoords;

void main() {
    outColor = texture(skybox, normalize(texCoords));
}