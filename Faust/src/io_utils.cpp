
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

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
			"C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\assets\\CornellBox\\CornellBox-Sphere.obj",
			"C:\\Users\\evaka\\Documents\\Visual Studio 2022\\Projects\\Faust\\Faust\\assets\\CornellBox")) {
			throw std::runtime_error(warn + err);
		}

		for (size_t shapeIdx = 0; shapeIdx < shapes.size(); ++shapeIdx) {
			const auto& shape = shapes[shapeIdx];
			const auto& mesh = shape.mesh;

			size_t indexOffset = 0;
			for (size_t faceIdx = 0; faceIdx < mesh.num_face_vertices.size(); ++faceIdx) {
				int matIndex = mesh.material_ids[faceIdx];
				const auto& mat = materials[matIndex];

				// Compute Face
				Face face;
				face.diffuse = { mat.diffuse[0], mat.diffuse[1], mat.diffuse[2] };
				face.emission = { mat.emission[0], mat.emission[1], mat.emission[2] };
				face.specular = { mat.specular[0], mat.specular[1], mat.specular[2] };
				face.roughness = std::sqrt(2.0f / (mat.shininess + 2.0f));
				face.ior = mat.ior > 0.0f ? mat.ior : 1.0f;

				bool isDielectric =
					(mat.dissolve < 0.99f || glm::length(glm::vec3(mat.transmittance[0], mat.transmittance[1], mat.transmittance[2])) > 0.01f)
					&& glm::length(glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2])) < 0.1f;

				bool isMirror = glm::length(glm::vec3(face.specular)) > 0.8f && glm::length(glm::vec3(face.diffuse)) < 0.1f;

				if (isDielectric) face.type = static_cast<float>(Material::Dielectric);
				else if (isMirror) face.type = static_cast<float>(Material::Mirror);
				else face.type = static_cast<float>(Material::Diffuse);

				// Push triangle: 3 vertices and 1 face
				for (size_t v = 0; v < 3; ++v) {
					const auto& index = mesh.indices[indexOffset + v];
					Vertex vertex{};
					vertex.meshId = shapeIdx;

					// Position
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						-attrib.vertices[3 * index.vertex_index + 1], // Flipping Y
						attrib.vertices[3 * index.vertex_index + 2]
					};

					// Normal (if available)
					if (index.normal_index >= 0) {
						vertex.normal = {
							attrib.normals[3 * index.normal_index + 0],
							-attrib.normals[3 * index.normal_index + 1], // Flip Y to match position
							attrib.normals[3 * index.normal_index + 2]
						};
					}
					else {
						vertex.normal = glm::vec3(0.0f); // Default/fallback
					}

					vertices.push_back(vertex);
					indices.push_back(static_cast<uint32_t>(indices.size()));
				}

				faces.push_back(face); // One face per triangle
				indexOffset += 3;
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

