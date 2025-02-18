#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) flat in int shadingSetting;

layout(location = 0) out vec4 outColor;

const vec3 lightDir = normalize(vec3(1.0, -3.0, 1.0));

void main() {
    switch (shadingSetting) {
    case 0:
        outColor = vec4(fragNormal, 1.0);
        break;
    case 1:
        outColor = texture(texSampler, fragTexCoord);
        break;
    default:
		float lightIntensity = max(dot(fragNormal, lightDir), 0);
		outColor = vec4(lightIntensity * fragColor, 1.0);
        break;
    }
    //outColor = vec4(fragColor, 1.0);

}