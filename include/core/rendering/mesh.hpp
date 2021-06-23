#pragma once
#include <vector>

#include "glm/glm.hpp"

#include "core/rendering/texture.hpp"
#include "core/resources.hpp"

#include "core/rendering/buffer.hpp"
#include "core/rendering/vertex_array.hpp"

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
                : vertices(std::move(verts)) ,
                  indices(std::move(ind)),
                  textures(std::move(tex))
          {
            va.bind();

            vb = VertexBuffer{vertices.size() * sizeof(Vertex)};
            vb.bind();
            vb.set(vertices.data(), vertices.size() * sizeof(Vertex));

            ib = IndexBuffer{indices.size() * sizeof(indices[0])};
            ib.bind();
            ib.set(indices.data(), indices.size() * sizeof(indices[0]));

            // vertex positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
            // vertex normals
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
            // vertex texture coords
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex_coord)));
            // vertex tangent
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tangent)));

            va.unbind();
        }

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) { *this = std::move(other); }

        Mesh& operator=(Mesh&& other) {
            std::swap(other.va, va);
            std::swap(other.vb, vb);
            std::swap(other.ib, ib);
            std::swap(other.vertices, vertices);
            std::swap(other.indices, indices);
            std::swap(other.textures, textures);
            return *this;
        }

        void draw(const Program &program) const
        {
            auto & textures_map = Resources::get<Texture2D>();
            size_t diffuse_num{}, specular_num{}, normal_num{};

            for(size_t i = 0; i < textures.size(); i++)
            {
                auto & type = textures_map[textures[i]].type;
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i));
                glBindTexture(GL_TEXTURE_2D, textures_map[textures[i]].id);

                if (type == Texture2D::Type::Diffuse) {
                    program.set_uniform(fmt::format("material.diffuse_{}", diffuse_num++), int(i));
                } else if(type == Texture2D::Type::Specular){
                    program.set_uniform(fmt::format("material.specular_{}", specular_num++), int(i));
                } else if(type == Texture2D::Type::Normal){
                    program.set_uniform(fmt::format("material.normal_{}", normal_num++), int(i));
                }

            }
            glActiveTexture(GL_TEXTURE0);

            // draw mesh
            va.bind();
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
            va.unbind();
        }
    };
}