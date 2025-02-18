#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    int shadingSetting;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;

layout(location = 3) flat out int shadingSetting;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    vec3 normalWorldSpace = normalize(mat3(ubo.model) * inNormal);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = normalWorldSpace;
    shadingSetting = ubo.shadingSetting;
}
