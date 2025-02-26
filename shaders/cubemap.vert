#version 450

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

vec2 positions[3] = vec2[3](
    vec2(-1.0, -1.0),  
    vec2( 3.0, -1.0),  
    vec2(-1.0,  3.0)   
);

layout(location = 0) out vec3 outUVW;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    mat4 viewNoTranslate = mat4(mat3(ubo.view));
    vec4 viewDir = normalize(inverse(ubo.proj) * vec4(positions[gl_VertexIndex], -1.0, 1.0));
    outUVW = vec3(normalize(inverse(viewNoTranslate) * viewDir));
}