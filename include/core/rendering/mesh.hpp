#pragma once
#include <vector>

#include "glm/glm.hpp"

#include "core/rendering/texture.hpp"
#include "core/resource_container.hpp"

namespace k2 {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coord;
    };

    class Mesh {
        GLuint vao{}, vbo{}, ebo{};
    public:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<uint64_t> textures;

        Mesh() = default;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<uint64_t> textures)
                : vertices(std::move(vertices)) ,
                  indices(std::move(indices)),
                  textures(std::move(textures)) {
            load();
        }

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) { *this = std::move(other); }

        Mesh& operator=(Mesh&& other) {
            std::swap(other.vao, vao);
            std::swap(other.vbo, vbo);
            std::swap(other.ebo, ebo);
            std::swap(other.vertices, vertices);
            std::swap(other.indices, indices);
            std::swap(other.textures, textures);
            return *this;
        }

        void draw(const Program &program, const ResourceContainer<Texture>& textures_map) const
        {
            size_t diffuse_num{}, specular_num{};

            for(size_t i = 0; i < textures.size(); i++)
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + i));
                glBindTexture(GL_TEXTURE_2D, textures_map[textures[i]].id);

                auto & type = textures_map[textures[i]].type;
                if (type == Texture::Type::Diffuse) {
                    program.set_uniform(fmt::format("material.diffuse_{}", diffuse_num++), float(i));
                } else if(type == Texture::Type::Specular){
                    program.set_uniform(fmt::format("material.specular_{}", specular_num++), float(i));
                }

            }
            glActiveTexture(GL_TEXTURE0);

            // draw mesh
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        ~Mesh() {
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
            glDeleteVertexArrays(1, &vao);
        }

        Mesh& load()
        {
            if (vao == 0){
                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vbo);
                glGenBuffers(1, &ebo);

                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);

                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                             indices.data(), GL_STATIC_DRAW);

                // vertex positions
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
                // vertex normals
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
                // vertex texture coords
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex_coord)));

                glBindVertexArray(0);
            }
            return *this;
        }
    };
}