#version 450

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

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

const float M_PI = 3.1415926538;

void main() {
  float dis = sqrt(dot(fragOffset, fragOffset));
  if (dis >= 0.25) {
    discard;
  }

  float cosDis = 0.25 * (cos(dis * M_PI) + 1);
  outColor = vec4(ubo.pointLight.color.xyz + 0.5 * cosDis, cosDis);
}