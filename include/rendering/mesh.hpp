#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>

#include "buffer.hpp"
#include "core/resources.hpp"
#include "core/utils.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"

namespace k2 {
class Mesh {
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;

public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 tex_coord;

        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal
                && tex_coord == other.tex_coord;
        }
    };

    struct Material {
        ResourceID albedo;
        ResourceID metallic;
        ResourceID roughness;
        ResourceID normal;
        ResourceID ambient_occlusion;

        glm::vec3 albedo_value;
        float metallic_value;
        float roughness_value;
    };

    struct MaterialGroup {
        Material material;
        std::vector<unsigned int> indices;
    };

    std::vector<Vertex> vertices;

    std::vector<Material> materials;
    std::vector<MaterialGroup> material_groups;

    Mesh() = default;

    Mesh(std::vector<Vertex>&& vertices_, std::vector<Material>&& materials_,
        std::vector<MaterialGroup>&& material_groups_)
        : vertices(std::move(vertices_))
        , materials(materials_)
        , material_groups(material_groups_) {
        generate_buffers();
        generate_vertex_array();
    }

    Mesh(const Mesh&) = delete;

    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept { *this = std::move(other); }

    Mesh& operator=(Mesh&& other) noexcept {
        std::swap(other.va, va);
        std::swap(other.vb, vb);
        std::swap(other.vertices, vertices);
        std::swap(other.materials, materials);
        std::swap(other.material_groups, material_groups);
        return *this;
    }

    void draw(const Program& program, ResourceManager& resources) const {
        size_t basevertex = 0;
        for (auto& group : material_groups) {
            auto& material = group.material;

            bind_textures(material, resources);
            setup_shader(program, material, resources);

            va.bind();
            glDrawElementsBaseVertex(GL_TRIANGLES, static_cast<GLsizei>(group.indices.size()), GL_UNSIGNED_INT, 0,
                static_cast<GLint>(basevertex));
            k2::VertexArray::unbind();

            basevertex += group.indices.size();
        }
    }

private:
    static void bind_textures(const Material& material, ResourceManager& resources) {
        auto& textures_map = resources.all<Texture2D>();
        if (textures_map.contains(material.albedo))
            textures_map[material.albedo].bind(0);
        if (textures_map.contains(material.metallic))
            textures_map[material.metallic].bind(1);
        if (textures_map.contains(material.roughness))
            textures_map[material.roughness].bind(2);
        if (textures_map.contains(material.normal))
            textures_map[material.normal].bind(3);
        if (textures_map.contains(material.ambient_occlusion))
            textures_map[material.ambient_occlusion].bind(4);
    }

    static void setup_shader(const Program& program, const Material& material, ResourceManager& resources) {
        auto& textures_map = resources.all<Texture2D>();
        program.set_uniform("material.albedo", 0);
        program.set_uniform("material.metallic", 1);
        program.set_uniform("material.roughness", 2);
        program.set_uniform("material.normal", 3);
        program.set_uniform("material.ambient_occlusion", 4);

        program.set_uniform("material.has_albedo", textures_map.contains(material.albedo));
        program.set_uniform("material.has_metallic", textures_map.contains(material.metallic));
        program.set_uniform("material.has_roughness", textures_map.contains(material.roughness));
        program.set_uniform("material.has_normal", textures_map.contains(material.normal));
        program.set_uniform("material.has_ambient_occlusion", textures_map.contains(material.ambient_occlusion));

        program.set_uniform("material.albedo_value", material.albedo_value);
        program.set_uniform("material.metallic_value", material.metallic_value);
        program.set_uniform("material.roughness_value", material.roughness_value);
    }

    void generate_buffers() {
        // Vertex buffer
        vb = VertexBuffer { vertices.size() * sizeof(Vertex) };
        vb.set(vertices.data(), vertices.size() * sizeof(Vertex));

        // Index buffer
        constexpr auto index_buffer_element_size = sizeof(material_groups.back().indices.back());

        auto total_indices = std::accumulate(material_groups.begin(), material_groups.end(), size_t(0),
            [](size_t acc, const auto& group) { return acc + group.indices.size(); });

        ib = IndexBuffer { total_indices * index_buffer_element_size };

        size_t offset = 0;
        for (auto& group : material_groups) {
            ib.set(group.indices.data(), group.indices.size() * index_buffer_element_size, offset);
            offset += group.indices.size() * index_buffer_element_size;
        }
    }

    void generate_vertex_array() {
        va.apply(
            {
                {
                    .buffer = vb.get(),
                    .attribute_trait {
                        .location = 0,
                        .data_type = ShaderDataType::Float3,
                        .offset = offsetof(Vertex, position),
                    },
                },
                {
                    .buffer = vb.get(),
                    .attribute_trait {
                        .location = 1,
                        .data_type = ShaderDataType::Float3,
                        .offset = offsetof(Vertex, color),
                    },
                },
                {
                    .buffer = vb.get(),
                    .attribute_trait {
                        .location = 2,
                        .data_type = ShaderDataType::Float3,
                        .offset = offsetof(Vertex, normal),
                    },
                },
                {
                    .buffer = vb.get(),
                    .attribute_trait {
                        .location = 3,
                        .data_type = ShaderDataType::Float2,
                        .offset = offsetof(Vertex, tex_coord),
                    },
                },
            },
            sizeof(Vertex), ib.get());
    }
};
}

namespace std {
template <> struct hash<k2::Mesh::Vertex> {
    size_t operator()(k2::Mesh::Vertex const& vertex) const {
        auto hash_first = std::hash<glm::vec3> {}(vertex.position);
        auto hash_second = std::hash<glm::vec3> {}(vertex.color);
        auto hash_third = std::hash<glm::vec3> {}(vertex.normal);
        auto hash_fourth = std::hash<glm::vec2> {}(vertex.tex_coord);

        auto hash = k2::hash_combine(hash_first, hash_second);
        hash = k2::hash_combine(hash, hash_third);
        hash = k2::hash_combine(hash, hash_fourth);
        return hash;
    }
};
}
