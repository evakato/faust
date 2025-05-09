#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex {
	glm::vec3 position;
	float pad0;           // <- manually align to 16 bytes

	glm::vec3 normal;
	float pad1;           // <- manually align to 16 bytes

	uint32_t meshId;
	float pad2[3];        // <- total size becomes 48 bytes, fully aligned

	static vk::VertexInputBindingDescription getBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription.setBinding(0);  // Binding 0
		bindingDescription.setStride(sizeof(Vertex));  // Stride = size of VertexRasterization
		bindingDescription.setInputRate(vk::VertexInputRate::eVertex);  // Per vertex

		return bindingDescription;
	}

	// Vertex Attribute Descriptions
	static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);
		attributeDescriptions[0].setLocation(0);
		attributeDescriptions[0].setBinding(0);
		attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
		attributeDescriptions[0].setOffset(offsetof(Vertex, position));

		attributeDescriptions[1].setLocation(1);
		attributeDescriptions[1].setBinding(0);
		attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
		attributeDescriptions[1].setOffset(offsetof(Vertex, normal));

		attributeDescriptions[2].setLocation(2);
		attributeDescriptions[2].setBinding(0);
		attributeDescriptions[2].setFormat(vk::Format::eR32Uint);
		attributeDescriptions[2].setOffset(offsetof(Vertex, meshId));

		return attributeDescriptions;
	}
};

enum class Material {
	Diffuse = 0,
	Mirror = 1,
	Dielectric = 2
};


struct Face {
	glm::vec3 diffuse;
	glm::vec3 emission;
	glm::vec3 specular;
	float roughness;
	float ior;
	float type;
};

namespace faust {
	inline void computeNormals(std::vector<Vertex>& vertices, std::vector<uint32_t> indices) {
		// Step 1: Zero out all normals
		for (auto& vertex : vertices) {
			vertex.normal[0] = vertex.normal[1] = vertex.normal[2] = 0.0f;
		}

		// Step 2: Compute face normals and accumulate
		for (size_t i = 0; i < indices.size(); i += 3) {
			uint32_t i0 = indices[i];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			glm::vec3 p0(vertices[i0].position[0], vertices[i0].position[1], vertices[i0].position[2]);
			glm::vec3 p1(vertices[i1].position[0], vertices[i1].position[1], vertices[i1].position[2]);
			glm::vec3 p2(vertices[i2].position[0], vertices[i2].position[1], vertices[i2].position[2]);

			glm::vec3 edge1 = p1 - p0;
			glm::vec3 edge2 = p2 - p0;

			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

			for (uint32_t idx : {i0, i1, i2}) {
				vertices[idx].normal[0] += normal.x;
				vertices[idx].normal[1] += normal.y;
				vertices[idx].normal[2] += normal.z;
			}
		}

		// Step 3: Normalize all normals
		for (auto& vertex : vertices) {
			glm::vec3 n(vertex.normal[0], vertex.normal[1], vertex.normal[2]);
			n = glm::normalize(n);
			vertex.normal[0] = n.x;
			vertex.normal[1] = n.y;
			vertex.normal[2] = n.z;
		}
	}
}
