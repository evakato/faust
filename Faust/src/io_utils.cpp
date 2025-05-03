
#include "io_utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>

namespace faust {
	void loadFromFile(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Face>& faces) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\assets\\CornellBox-Original.obj"
			, "C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\assets")) {
			throw std::runtime_error(warn + err);
		}

		std::cout << shapes.size() << std::endl;

		for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++) {
			const auto& shape = shapes[shapeIdx];
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};
				vertex.meshId = shapeIdx;
				vertex.position[0] = attrib.vertices[3 * index.vertex_index + 0];
				vertex.position[1] = -attrib.vertices[3 * index.vertex_index + 1];
				vertex.position[2] = attrib.vertices[3 * index.vertex_index + 2];
				// Normals
				if (index.normal_index >= 0) {
					/*
					vertex.normal[0] = attrib.normals[3 * index.normal_index + 0];
					vertex.normal[1] = -attrib.normals[3 * index.normal_index + 1];
					vertex.normal[2] = attrib.normals[3 * index.normal_index + 2];
					*/
				}

				vertices.push_back(vertex);
				indices.push_back(static_cast<uint32_t>(indices.size()));
			}
			for (const auto& matIndex : shape.mesh.material_ids) {
				Face face;
				face.diffuse[0] = materials[matIndex].diffuse[0];
				face.diffuse[1] = materials[matIndex].diffuse[1];
				face.diffuse[2] = materials[matIndex].diffuse[2];
				face.emission[0] = materials[matIndex].emission[0];
				face.emission[1] = materials[matIndex].emission[1];
				face.emission[2] = materials[matIndex].emission[2];
				faces.push_back(face);
			}
		}
	}

	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
}

