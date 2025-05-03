#version 450

layout(set = 0, binding = 0, std140) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 prevView;
    mat4 prevProj;
} cam;

layout(location = 0) in vec3 normal;  
layout(location = 1) flat in uint meshId; 

layout(location = 0) out vec3 fragNormal;  
layout(location = 1) out vec2 fragMotion;
layout(location = 2) out uint fragMeshId;
layout(location = 3) out float fragDepth;

vec3 recoverWorldPos(vec3 fragCoord, mat4 proj, mat4 view) {
    vec2 ndcXY = (fragCoord.xy / vec2(1024, 1024)) * 2.0 - 1.0;
    float depth = fragCoord.z * 2.0 - 1.0;
    vec4 clipSpace = vec4(ndcXY, depth, 1.0);

    mat4 invViewProj = inverse(proj * view);
    vec4 worldPos = invViewProj * clipSpace;
    return worldPos.xyz / worldPos.w;
}

void main() {
    vec3 worldPos = recoverWorldPos(gl_FragCoord.xyz, cam.proj, cam.view);
    vec4 prevClip = cam.prevProj * cam.prevView * vec4(worldPos, 1.0);
    prevClip /= prevClip.w;
    vec2 prevUV = prevClip.xy * 0.5 + 0.5;
    vec2 currentUV = gl_FragCoord.xy / vec2(1024,1024);

    fragMotion = currentUV - prevUV;
    fragNormal = normalize(normal);
    fragMeshId = meshId;
    fragDepth = gl_FragCoord.z;
}
