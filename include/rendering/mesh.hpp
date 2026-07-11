#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <utility>
#include <vector>

#include "buffer.hpp"
#include "core/resources.hpp"
#include "core/utils.hpp"
#include "shader.hpp"
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
        std::vector<MaterialGroup>&& material_groups_);

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

    void draw(const Program& program, ResourceManager& resources) const;

private:
    static void bind_textures(const Material& material, ResourceManager& resources);

    static void setup_shader(const Program& program, const Material& material, ResourceManager& resources);

    void generate_buffers();

    void generate_vertex_array();
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
