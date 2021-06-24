#pragma once

#include "core/resources.hpp"
#include "core/rendering/buffer.hpp"
#include "core/rendering/vertex_array.hpp"
#include "core/rendering/camera.hpp"


#include <unordered_map>
#include <unordered_set>

namespace k2 {
    class Renderer2D {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec4 color;
            glm::vec2 texture_coordinate;
            ResourceID texture;
        };
        Camera camera;
    private:
        Program default_shader = [](auto vertex, auto fragment) {
            namespace fs = std::filesystem;
            auto vertex_shader = k2::Shader(GL_VERTEX_SHADER, fs::path(vertex));
            if (!vertex_shader) { k2::Logger::core->critical(vertex_shader.error_msg().value()); }

            auto fragment_shader = k2::Shader(GL_FRAGMENT_SHADER, fs::path(fragment));
            if (!fragment_shader) { k2::Logger::core->critical(fragment_shader.error_msg().value()); }

            k2::Program ret{std::move(vertex_shader), std::move(fragment_shader)};
            ret.link();

            if (!ret) { k2::Logger::app->critical(ret.error_msg().value()); }
            return ret;
        }("res/shaders/2D_vs.glsl", "res/shaders/2D_fs.glsl");

        std::unordered_map<std::uint32_t, std::vector<Renderer2D::Vertex>> vertices_buffer{};
        std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> indices_buffer{};
        std::unordered_map<ResourceID, std::uint32_t> texture_unit_map{};

        std::size_t max_vertices = 100'000;
        std::uint32_t max_textures = 32;
        VertexArray vao;
        VertexBuffer vbo;
        IndexBuffer ebo;
    public:

        Renderer2D() {
            vbo = VertexBuffer{max_vertices * sizeof(Vertex)};
            ebo = IndexBuffer{max_vertices * sizeof(std::uint32_t)};
            vao.apply({
                              {
                                      .buffer = vbo.get(),
                                      .attribute_trait {
                                              .location = 0,
                                              .data_type = ShaderDataType::Float3,
                                              .offset = offsetof(Vertex, position),
                                      },
                              },
                              {
                                      .buffer = vbo.get(),
                                      .attribute_trait {
                                              .location = 1,
                                              .data_type = ShaderDataType::Float4,
                                              .offset = offsetof(Vertex, color),
                                      },
                              },
                              {
                                      .buffer = vbo.get(),
                                      .attribute_trait {
                                              .location = 2,
                                              .data_type = ShaderDataType::Float2,
                                              .offset = offsetof(Vertex, texture_coordinate),
                                      },
                              },
                              {
                                      .buffer = vbo.get(),
                                      .attribute_trait{
                                              .location = 3,
                                              .data_type = ShaderDataType::Uint,
                                              .offset = offsetof(Vertex, texture),
                                      },
                              }
                      }, sizeof(Vertex), ebo.get());
        }

        Renderer2D(const Renderer2D &) = delete;

        Renderer2D &operator=(const Renderer2D &) = delete;

        /*
         * Batch rendering
         * deferred + forward rendering
         * supports different textures, normal maps, light sources, directional and ambient light
         * color of vertices/textures
         * supports drawing to a render target
         * shadows?
         */
        void draw(std::span<const Renderer2D::Vertex> vertices, std::span<const std::uint32_t> indices,
                  const glm::mat3 &transform = glm::mat4(1.0f), const std::uint32_t draw_mode = GL_TRIANGLES) {
            std::unordered_set<ResourceID> textures_new{};
            for (const auto &vertex: vertices) { textures_new.insert(vertex.texture); }
            size_t total_textures = texture_unit_map.size();
            for (auto &i: textures_new) { total_textures += !texture_unit_map.count(i); }


            auto &vertices_vec = vertices_buffer[draw_mode];
            auto &indices_vec = indices_buffer[draw_mode];

            if (total_textures > max_textures
                || vertices_vec.size() + vertices.size() > max_vertices
                || indices_vec.size() + indices.size() > max_vertices) {
                render();
            }

            // Assign the new textures, texture unit coordinates
            for (auto &i: textures_new) {
                if (!texture_unit_map.count(i)) {
                    auto next_index = texture_unit_map.size();
                    texture_unit_map[i] = static_cast<std::uint32_t>(next_index);
                }
            }

            for (auto vertex: vertices) {
                vertex.position = glm::vec3(transform * glm::vec4(vertex.position, 1.0f));
                vertex.texture = texture_unit_map[vertex.texture];
                vertices_vec.push_back(vertex);
            }
            indices_vec.insert(indices_vec.end(), indices.begin(), indices.end());
        }

        void render() {
            for (const auto &[draw_mode, vertices]: vertices_buffer) {
                auto &indices = indices_buffer[draw_mode];

                std::vector<std::uint32_t> tex_unit_vec(texture_unit_map.size());
                std::iota(tex_unit_vec.begin(), tex_unit_vec.end(), 0);

                for (auto &[texture_id, texture_unit_index] : texture_unit_map) {
                    [[maybe_unused]] auto &texture = Resources::get<Texture2D>(texture_id);
                    texture.bind(texture_unit_index);
                }

                default_shader.use()
                        .set_uniform("texture_list", std::span{tex_unit_vec.data(), tex_unit_vec.size()})
                        .set_uniform("model", glm::mat4(1.0f))
                        .set_uniform("view", camera.get_view())
                        .set_uniform("projection", camera.get_projection());

                vao.bind();
                vbo.set(vertices.data(), sizeof(vertices[0]) * vertices.size());
                ebo.set(indices.data(), sizeof(indices[0]) * indices.size());
                glDrawElements(draw_mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
                vao.unbind();
            }

            texture_unit_map.clear();
            for (auto &[draw_mode, vertices] : vertices_buffer) { vertices.clear(); }
            for (auto &[draw_mode, indices] : indices_buffer) { indices.clear(); }
        }


    };
}