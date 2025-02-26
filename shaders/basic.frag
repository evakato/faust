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
    mat4 invView;
    vec4 directionalLight;
    PointLight pointLight;
    AmbientLight ambientLight;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube skybox;

void main() {
    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDir = normalize(cameraPosWorld - fragPos);

    switch (int(ubo.directionalLight.w)) {
    case 0:
        outColor = vec4(fragNormal, 1.0);
        break;
    case 1:
        outColor = texture(texSampler, fragTexCoord);
        break;
    case 2:
    case 3:
    case 4:
        vec3 ambient = ubo.ambientLight.color.xyz * ubo.ambientLight.color.w;
        vec3 surfaceNormal = normalize(fragNormal);

        vec3 directionToPointLight = ubo.pointLight.position.xyz - fragPos;
        float attenuation = 1.0 / dot(directionToPointLight, directionToPointLight);
        directionToPointLight = normalize(directionToPointLight);

        float incidenceAngle = max(dot(surfaceNormal, directionToPointLight), 0);
        vec3 intensity = ubo.pointLight.color.xyz * ubo.pointLight.color.w * attenuation;

        vec3 diffuseLight = ambient + intensity * incidenceAngle;
        
        if (int(ubo.directionalLight.w) == 2) {
            outColor = vec4(diffuseLight * fragColor, 1.0);
            break;
        }
        
        vec3 halfAngle = normalize(directionToPointLight + viewDir);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
	    vec3 specularLight = intensity * blinnTerm;

        if (int(ubo.directionalLight.w) == 3) {
			outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
			break;
        }

        vec3 reflection = reflect(-viewDir, normalize(fragNormal));
        vec3 refraction = refract(-viewDir, normalize(fragNormal), 1.00 / 1.52);
        vec3 envColor = texture(skybox, reflection).rgb;

        outColor = vec4(ambient + diffuseLight * envColor + specularLight, 1.0);
        outColor = vec4(envColor, 1.0);
        break;
    }
}