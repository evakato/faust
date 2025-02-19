#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

struct PointLight {
    vec4 color;
    vec4 position;
};

struct AmbientLight {
    vec4 color;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normalMat;
    vec4 directionalLight;
    PointLight pointLight;
    AmbientLight ambientLight;
} ubo;
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    switch (int(ubo.directionalLight.w)) {
    case 0:
        outColor = vec4(fragNormal, 1.0);
        break;
    case 1:
        outColor = texture(texSampler, fragTexCoord);
        break;
    default:
        vec3 directionToPointLight = ubo.pointLight.position.xyz - fragPos;
        float attenuation = 1.0 / dot(directionToPointLight, directionToPointLight);

        vec3 pointLightColor = ubo.pointLight.color.xyz * ubo.pointLight.color.w * attenuation;
        vec3 ambient = ubo.ambientLight.color.xyz * ubo.ambientLight.color.w;

		vec3 diffuseLight = pointLightColor * max(dot(fragNormal, directionToPointLight), 0);

		outColor = vec4((diffuseLight + ambient)* fragColor, 1.0);
        break;
    }
}