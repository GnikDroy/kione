#pragma once

#include "core/resources.hpp"

#include "rendering/buffer.hpp"
#include "rendering/frame_buffer.hpp"
#include "rendering/vertex_array.hpp"

#include "components/camera.hpp"
#include "components/sprite.hpp"
#include "components/transform.hpp"
#include "core/scene.hpp"

#include <numeric>
#include <span>
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
        if (!vertex_shader) {
            k2::Log::core().critical(vertex_shader.error_msg().value());
        }

        auto fragment_shader = k2::Shader(GL_FRAGMENT_SHADER, fs::path(fragment));
        if (!fragment_shader) {
            k2::Log::core().critical(fragment_shader.error_msg().value());
        }

        k2::Program ret { std::move(vertex_shader), std::move(fragment_shader) };
        ret.link();

        if (!ret) {
            k2::Log::app().critical(ret.error_msg().value());
        }
        return ret;
    }("res/shaders/2D_vs.glsl", "res/shaders/2D_fs.glsl");

    std::unordered_map<std::uint32_t, std::vector<Renderer2D::Vertex>> vertices_buffer {};
    std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> indices_buffer {};
    std::unordered_map<ResourceID, std::uint32_t> texture_unit_map {};

    std::size_t max_vertices = 100'000;
    std::uint32_t max_textures = 32;
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ebo;
    FrameBuffer frame_buffer;

public:
    Renderer2D() {
        vbo = VertexBuffer { max_vertices * sizeof(Vertex) };
        ebo = IndexBuffer { max_vertices * sizeof(std::uint32_t) };
        vao.apply({ {
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
                          .attribute_trait {
                              .location = 3,
                              .data_type = ShaderDataType::Uint,
                              .offset = offsetof(Vertex, texture),
                          },
                      } },
            sizeof(Vertex), ebo.get());
    }

    Renderer2D(const Renderer2D&) = delete;

    Renderer2D& operator=(const Renderer2D&) = delete;

    FrameBuffer& set_frame_buffer(FrameBuffer&& fb) { return frame_buffer = std::move(fb); }

    FrameBuffer& get_frame_buffer() { return frame_buffer; }

    void clear(std::uint32_t mask = GL_COLOR_BUFFER_BIT) {
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.get_id());
        glClear(mask);
        if (!frame_buffer.is_swap_chain_target()) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void set_clear_color(float r, float g, float b, float a) { glClearColor(r, g, b, a); }

    /*
     * TODO: deferred lighting + forward renderer
     * TODO: light sources, directional and ambient light
     * TODO: shadows?
     */
    void draw(std::span<const Renderer2D::Vertex> vertices, std::span<const std::uint32_t> indices,
        const glm::mat4& transform = glm::mat4(1.0f), const std::uint32_t draw_mode = GL_TRIANGLES) {
        std::unordered_set<ResourceID> textures_new {};
        for (const auto& vertex : vertices) {
            textures_new.insert(vertex.texture);
        }
        size_t total_textures = texture_unit_map.size();
        for (auto& i : textures_new) {
            total_textures += !texture_unit_map.count(i);
        }

        auto& vertices_vec = vertices_buffer[draw_mode];
        auto& indices_vec = indices_buffer[draw_mode];

        if (total_textures > max_textures || vertices_vec.size() + vertices.size() > max_vertices
            || indices_vec.size() + indices.size() > max_vertices) {
            render();
        }

        // Assign the new textures, texture unit coordinates
        for (auto& i : textures_new) {
            if (!texture_unit_map.count(i)) {
                auto next_index = texture_unit_map.size();
                texture_unit_map[i] = static_cast<std::uint32_t>(next_index);
            }
        }

        for (auto vertex : vertices) {
            vertex.position = glm::vec3(transform * glm::vec4(vertex.position, 1.0f));
            vertex.texture = texture_unit_map[vertex.texture];
            vertices_vec.push_back(vertex);
        }

        std::ranges::transform(indices, std::back_inserter(indices_vec),
            [offset = vertices_vec.size() - vertices.size()](auto& i) { return i + std::uint32_t(offset); });
    }

    void draw(const TransformComponent& transform, const SpriteComponent& sprite) {
        std::array<k2::Renderer2D::Vertex, 4> vertices {
            k2::Renderer2D::Vertex {
                .position = { 1.0f, -1.0f, 0.0f },
                .color = sprite.color,
                .texture_coordinate = { sprite.uv_rect.x + sprite.uv_rect.w, sprite.uv_rect.y },
                .texture = sprite.texture,
            },
            k2::Renderer2D::Vertex {
                .position = { -1.0f, 1.0f, 0.0f },
                .color = sprite.color,
                .texture_coordinate = { sprite.uv_rect.x, sprite.uv_rect.y + sprite.uv_rect.h },
                .texture = sprite.texture,
            },
            k2::Renderer2D::Vertex {
                .position = { -1.0f, -1.0f, 0.0f },
                .color = sprite.color,
                .texture_coordinate = { sprite.uv_rect.x, sprite.uv_rect.y },
                .texture = sprite.texture,
            },
            k2::Renderer2D::Vertex {
                .position = { 1.0f, 1.0f, 0.0f },
                .color = sprite.color,
                .texture_coordinate = { sprite.uv_rect.x + sprite.uv_rect.w, sprite.uv_rect.y + sprite.uv_rect.h },
                .texture = sprite.texture,
            },
        };
        const std::array<std::uint32_t, 6> indices { 0, 1, 2, 0, 3, 1 };
        draw(vertices, indices, transform.get_matrix());
    }

    void draw(const Scene& scene) {
        // TODO: how to select main camera here?
        // TODO: do i even select the main camera here?
        scene.registry.view<k2::Camera>().each([&](auto, const auto& cam) { camera = cam; });

        scene.registry.view<k2::TransformComponent, k2::SpriteComponent>().each(
            [&](auto, const auto& transform, const auto& sprite) { draw(transform, sprite); });
    }

    void render() {
        for (const auto& [draw_mode, vertices] : vertices_buffer) {
            auto& indices = indices_buffer[draw_mode];

            std::vector<std::int32_t> tex_unit_vec(texture_unit_map.size());
            std::iota(tex_unit_vec.begin(), tex_unit_vec.end(), 0);

            for (auto& [texture_id, texture_unit_index] : texture_unit_map) {
                [[maybe_unused]] auto& texture = Resources::get<Texture2D>(texture_id);
                texture.bind(texture_unit_index);
            }

            default_shader.use()
                .set_uniform("texture_list", std::span { tex_unit_vec.data(), tex_unit_vec.size() })
                .set_uniform("model", glm::mat4(1.0f))
                .set_uniform("view", camera.get_view())
                .set_uniform("projection", camera.get_projection());

            vao.bind();
            vbo.set(vertices.data(), sizeof(vertices[0]) * vertices.size());
            ebo.set(indices.data(), sizeof(indices[0]) * indices.size());
            glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.get_id());
            glDrawElements(draw_mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
            if (!frame_buffer.is_swap_chain_target()) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
            vao.unbind();
        }

        texture_unit_map.clear();
        for (auto& [draw_mode, vertices] : vertices_buffer) {
            vertices.clear();
        }
        for (auto& [draw_mode, indices] : indices_buffer) {
            indices.clear();
        }
    }
};
}