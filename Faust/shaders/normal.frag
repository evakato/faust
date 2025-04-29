#version 450

layout(location = 0) in vec3 normal;  // Normal from vertex shader

layout(location = 0) out vec3 fragNormal;  
layout(location = 1) out float fragDepth;

void main() {
    fragNormal = normalize(normal);
    fragDepth = gl_FragCoord.z;
}
