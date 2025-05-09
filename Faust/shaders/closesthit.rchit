#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : enable
#include "common.glsl"

layout(binding = 2, set = 0) buffer Vertices{float vertices[];};
layout(binding = 3, set = 0) buffer Indices{uint indices[];};
layout(binding = 4, set = 0) buffer Faces{float faces[];};
layout(set = 0, binding = 5, std140) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 prevView;
    mat4 prevProj;
} cam;

layout(location = 0) rayPayloadInEXT HitPayload payload;
hitAttributeEXT vec3 attribs;

struct Vertex {
    vec3 position;
    vec3 normal;
    uint meshID;
};

struct Face {
    vec3 diffuse;
    vec3 emission;
    vec3 specular;
    float roughness;
    float ior;
    float type; // 0 = diffuse, 1 = mirror, 2 = dielectric
};


Vertex unpackVertex(uint index) {
    const uint stride = 12; // 12 floats = 48 bytes
    uint offset = index * stride;

    Vertex v;
    v.position = vec3(vertices[offset + 0], vertices[offset + 1], vertices[offset + 2]);
    // vertices[offset + 3] is padding

    v.normal = vec3(vertices[offset + 4], vertices[offset + 5], vertices[offset + 6]);
    // vertices[offset + 7] is padding

    v.meshID = floatBitsToUint(vertices[offset + 8]);
    // vertices[offset + 9], [10], [11] are padding

    return v;
}

Face unpackFace(uint index)
{
    uint stride = 12; // total floats per face
    uint offset = index * stride;

    Face f;
    f.diffuse  = vec3(faces[offset +  0], faces[offset + 1], faces[offset + 2]);
    f.emission = vec3(faces[offset +  3], faces[offset + 4], faces[offset + 5]);
    f.specular = vec3(faces[offset +  6], faces[offset + 7], faces[offset + 8]);
    f.roughness = faces[offset + 9];
    f.ior       = faces[offset + 10];
    f.type      = faces[offset + 11];
    return f;
}

void main()
{
    const Vertex v0 = unpackVertex(indices[3 * gl_PrimitiveID + 0]);
    const Vertex v1 = unpackVertex(indices[3 * gl_PrimitiveID + 1]);
    const Vertex v2 = unpackVertex(indices[3 * gl_PrimitiveID + 2]);

    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    const vec3 position = v0.position * barycentricCoords.x + v1.position * barycentricCoords.y + v2.position * barycentricCoords.z;
    const vec3 normal = normalize(v0.normal * barycentricCoords.x +
              v1.normal * barycentricCoords.y +
              v2.normal * barycentricCoords.z);

    const Face face = unpackFace(gl_PrimitiveID);

    payload.brdf = face.diffuse / M_PI;
    payload.emission = face.emission * 2.0;
    payload.position = position;
    payload.normal = normal;

    payload.materialType = int(face.type);
    payload.roughness = face.roughness;
    payload.ior = face.ior;
    payload.specular = face.specular;

    if (face.type == 0) {
        // Diffuse
        payload.brdf = face.diffuse / M_PI;
    } else if (face.type == 1) {
        // Mirror — pure specular handled in ray continuation
    } else if (face.type == 2) {
        // Dielectric — BRDF not evaluated here
    }
}
