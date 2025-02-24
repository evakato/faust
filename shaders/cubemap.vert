#version 450

layout (location = 0) in vec3 inPos;
layout(location = 0) out vec3 texCoords;

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


vec3 positions[8] = vec3[8](
    vec3(-1, -1, -1), vec3( 1, -1, -1),
    vec3( 1,  1, -1), vec3(-1,  1, -1),
    vec3(-1, -1,  1), vec3( 1, -1,  1),
    vec3( 1,  1,  1), vec3(-1,  1,  1)
);

void main() {
    // Indices for cube faces (no vertex buffer needed)
    int indices[36] = int[36](
        0, 1, 2, 2, 3, 0,  // -Z
        1, 5, 6, 6, 2, 1,  // +X
        7, 6, 5, 5, 4, 7,  // +Z
        4, 0, 3, 3, 7, 4,  // -X
        4, 5, 1, 1, 0, 4,  // -Y
        3, 2, 6, 6, 7, 3   // +Y
    );

    int vid = indices[gl_VertexIndex];
    vec3 pos = positions[vid];

    // Remove translation from view matrix
    mat4 viewNoTranslate = mat4(mat3(ubo.view));

    // Compute final position (w = 1, so no perspective divide)
    gl_Position = ubo.proj * viewNoTranslate * vec4(pos, 1.0);

    // Output texture coordinates (same as world direction)
    texCoords = pos;
}