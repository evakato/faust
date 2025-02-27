#include "model.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Model::Model(FaustDevice& device, std::string& modelPath) : device{ device } {
	setupNewModel(modelPath);
}

Model::~Model() {}

void Model::setupNewModel(std::string& modelPath) {
	vertices.clear();
	indices.clear();
	loadModel(modelPath);
	vertexBuffer.setupBuffer(sizeof(vertices[0]) * vertices.size(), (void*)vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	indexBuffer.setupBuffer(sizeof(indices[0]) * indices.size(), (void*)indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void Model::loadModel(std::string& modelPath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal = (index.normal_index >= 0) ?
				glm::vec3{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2] } :
				glm::vec3{ 0.0f, 0.0f, 0.0f };

			vertex.texCoord = (index.texcoord_index >= 0)
				? glm::vec2{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			}
			: glm::vec2{ 0.0f, 0.0f };

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void Model::bind(VkCommandBuffer commandBuffer) const {
	VkBuffer vertexBuffers[] = { vertexBuffer.getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}
