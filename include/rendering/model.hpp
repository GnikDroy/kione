#pragma once

#include "core/fnv.hpp"
#include "core/logger.hpp"
#include "core/resources.hpp"
#include "mesh.hpp"

#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

#include <filesystem>
#include <string>
#include <vector>
#include <numeric>

namespace k2 {
class Model {
    std::vector<Mesh> meshes;
    std::filesystem::path path;

public:
    Model() = default;

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    Model(const std::filesystem::path& path, ResourceManager& resources)
        : path { path } {
        auto success = load_model(resources);

        // Logging information of the model.
        k2::Log::core().trace(std::format("Loading model {} successful: {}", path.string(), success));
        auto num_meshes = meshes.size();
        auto num_vertices = std::accumulate(
            meshes.begin(), meshes.end(), size_t(0), [](auto n, const Mesh& mesh) { return n + mesh.vertices.size(); });
        k2::Log::core().trace(std::format("Model contains: {} meshes and {} vertices", num_meshes, num_vertices));
    }

    void draw(const Program& program, ResourceManager& resources) {
        for (const auto& mesh : meshes) {
            mesh.draw(program, resources);
        }
    }

private:
    bool load_model(ResourceManager& resources) {
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path.string())) {
            if (!reader.Error().empty()) {
                k2::Log::core().error(std::format("Failed to load model at {} : {}", path.string(), reader.Error()));
                return false;
            }
        }

        if (!reader.Warning().empty()) {
            k2::Log::core().warn(std::format("Model at {} has warnings: {}", path.string(), reader.Warning()));
        }

        return process_model(reader, resources);
    }

    bool process_model(const tinyobj::ObjReader& reader, ResourceManager& resources) {
        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();

        auto materials = process_materials(reader.GetMaterials(), resources);

        for (auto& shape : shapes) {
            std::unordered_map<Mesh::Vertex, uint32_t> unique_vertices;
            std::vector<Mesh::Vertex> vertices;
            std::vector<Mesh::MaterialGroup> material_groups;
            std::unordered_map<size_t, size_t> material_indices;

            size_t index_offset = 0;
            for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++) {
                assert(shape.mesh.num_face_vertices[face] == 3 && "Mesh not triangulated.");
                
                auto material_id = shape.mesh.material_ids[face];
                
                if (!material_indices.contains(material_id)) {
                    material_indices[material_id] = material_groups.size();
                    material_groups.push_back(Mesh::MaterialGroup {
                        .material = materials[material_id],
                        .indices = {}
                    });
                }
                
                auto& material_group = material_groups[material_indices[material_id]];

                for (size_t v = 0; v < 3; v++) {
                    auto index = shape.mesh.indices[index_offset + v];
                    Mesh::Vertex vertex {};

                    vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
                    vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
                    vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];

                    vertex.color.r = attrib.colors[3 * index.vertex_index + 0];
                    vertex.color.g = attrib.colors[3 * index.vertex_index + 1];
                    vertex.color.b = attrib.colors[3 * index.vertex_index + 2];

                    if (index.normal_index >= 0) {
                        vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
                        vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
                        vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
                    }

                    if (index.texcoord_index >= 0) {
                        vertex.tex_coord.x = attrib.texcoords[2 * index.texcoord_index + 0];
                        vertex.tex_coord.y = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
                        // The UV coordinates are flipped because OpenGL expects the origin to be at the bottom left.
                    }

                    if (!unique_vertices.contains(vertex)) {
                        unique_vertices[vertex] = (unsigned int)vertices.size();
                        vertices.push_back(vertex);
                    }

                    material_group.indices.push_back(unique_vertices[vertex]);
                }

                index_offset += 3;
            }

            meshes.emplace_back(std::move(vertices), std::move(materials), std::move(material_groups));
        }
        return true;
    }

    std::vector<Mesh::Material> process_materials(
        const std::vector<tinyobj::material_t>& materials_, ResourceManager& resources) {
        std::vector<Mesh::Material> materials;

        auto parent_path = path.parent_path();

        for (auto& material_ : materials_) {
            Mesh::Material material;
            material.albedo = get_material_texture(parent_path / material_.diffuse_texname, resources);
            material.metallic = get_material_texture(parent_path / material_.metallic_texname, resources);
            material.roughness = get_material_texture(parent_path / material_.roughness_texname, resources);
            material.normal = get_material_texture(parent_path / material_.normal_texname, resources);
            material.ambient_occlusion = get_material_texture(parent_path / material_.ambient_texname, resources);

            material.albedo_value = { material_.diffuse[0], material_.diffuse[1], material_.diffuse[2] };
            material.metallic_value = material_.metallic;
            material.roughness_value = material_.roughness;
            materials.push_back(material);
        }

        return materials;
    }

    static std::uint64_t get_material_texture(const std::filesystem::path& material_path, ResourceManager& resources) {
        auto path_str = material_path.string();
        if (!resources.contains<Texture2D>(fnv1a(path_str))) {
            auto image = Image(material_path);
            if (!image) {
                k2::Log::core().warn(std::format("Model material not found: {}", path_str));
                return fnv1a(path_str);
            }

            resources.set(path_str, Texture2D { image });
        }
        return fnv1a(path_str);
    }
};
}
