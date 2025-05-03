#version 450

layout(location = 0) in vec3 inPosition; 
layout(location = 1) in vec3 inNormal;   
layout(location = 2) in uint inMeshId;   

layout(set = 0, binding = 0, std140) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 prevView;
    mat4 prevProj;
} cam;

layout(location = 0) out vec3 fragNormal; 
layout(location = 1) flat out uint fragMeshId; 

void main() {
    mat4 modelMatrix = mat4(1.0);
    vec4 worldPos = modelMatrix * vec4(inPosition, 1.0);
    gl_Position = cam.proj * cam.view * worldPos;

    vec3 normalWorldSpace = normalize((transpose(inverse(modelMatrix)) * vec4(inNormal, 0.0)).xyz);
    fragNormal = normalWorldSpace;
    fragMeshId = inMeshId;
}
