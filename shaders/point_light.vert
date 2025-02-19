#version 450

layout (location = 0) out vec2 fragOffset;

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

const vec2 quadVertices[6] = vec2[](
    vec2(-0.5, -0.5), vec2( 0.5, -0.5), vec2( 0.5,  0.5), 
    vec2(-0.5, -0.5), vec2( 0.5,  0.5), vec2(-0.5,  0.5)
);

void main() {
    fragOffset = quadVertices[gl_VertexIndex];
  vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
  vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

  vec3 positionWorld = ubo.pointLight.position.xyz
    + 0.5 * fragOffset.x * cameraRightWorld
    + 0.5 * fragOffset.y * cameraUpWorld;

  gl_Position = ubo.proj * ubo.view * vec4(positionWorld, 1.0);
}