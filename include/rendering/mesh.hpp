#pragma once

#include <format>
#include <glm/glm.hpp>
#include <vector>

#include "buffer.hpp"
#include "core/resources.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"

namespace k2 {
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
    glm::vec3 tangent;
};

class Mesh {
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;

public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<uint64_t> textures;

    Mesh() = default;

    Mesh(std::vector<Vertex> verts, std::vector<unsigned int> ind, std::vector<uint64_t> tex)
        : vertices(std::move(verts))
        , indices(std::move(ind))
        , textures(std::move(tex)) {
        vb = VertexBuffer { vertices.size() * sizeof(Vertex) };
        vb.set(vertices.data(), vertices.size() * sizeof(Vertex));

        ib = IndexBuffer { indices.size() * sizeof(indices[0]) };
        ib.set(indices.data(), indices.size() * sizeof(indices[0]));

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
                        .offset = offsetof(Vertex, normal),
                    },
                },
                {
                    .buffer = vb.get(),
                    .attribute_trait {
                        .location = 2,
                        .data_type = ShaderDataType::Float2,
                        .offset = offsetof(Vertex, tex_coord),
                    },
                },
                {
                    .buffer = vb.get(),
                    .attribute_trait {
                        .location = 3,
                        .data_type = ShaderDataType::Float3,
                        .offset = offsetof(Vertex, tangent),
                    },
                },
            },
            sizeof(Vertex), ib.get());
    }

    Mesh(const Mesh&) = delete;

    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept { *this = std::move(other); }

    Mesh& operator=(Mesh&& other) noexcept {
        std::swap(other.va, va);
        std::swap(other.vb, vb);
        std::swap(other.ib, ib);
        std::swap(other.vertices, vertices);
        std::swap(other.indices, indices);
        std::swap(other.textures, textures);
        return *this;
    }

    void draw(const Program& program) const {
        auto& textures_map = Resources::get<Texture2D>();
        size_t diffuse_num {};
        size_t specular_num {};
        size_t normal_num {};

        for (size_t i = 0; i < textures.size(); i++) {
            auto& type = textures_map[textures[i]].type;
            textures_map[textures[i]].bind((uint32_t)i);

            if (type == Texture2D::Type::Diffuse) {
                program.set_uniform(std::format("material.diffuse_{}", diffuse_num++), int(i));
            } else if (type == Texture2D::Type::Specular) {
                program.set_uniform(std::format("material.specular_{}", specular_num++), int(i));
            } else if (type == Texture2D::Type::Normal) {
                program.set_uniform(std::format("material.normal_{}", normal_num++), int(i));
            }
        }

        // draw mesh
        va.bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        k2::VertexArray::unbind();
    }
};
}
