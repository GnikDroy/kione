#include "rendering/renderer2D.hpp"

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <unordered_set>

#include "core/logger.hpp"
#include "core/scene.hpp"

namespace k2 {

static Program load_default_shader(const char* vertex, const char* fragment) {
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
}

Renderer2D::Renderer2D()
    : default_shader { load_default_shader("res/shaders/2D_vs.glsl", "res/shaders/2D_fs.glsl") } {
    vbo = VertexBuffer { max_vertices * sizeof(VertexShaderInput) };
    ebo = IndexBuffer { max_vertices * sizeof(std::uint32_t) };
    vao.apply({ {
                    .buffer = vbo.get(),
                    .attribute_trait {
                        .location = 0,
                        .data_type = ShaderDataType::Float3,
                        .offset = offsetof(VertexShaderInput, position),
                    },
                },
                  {
                      .buffer = vbo.get(),
                      .attribute_trait {
                          .location = 1,
                          .data_type = ShaderDataType::Float4,
                          .offset = offsetof(VertexShaderInput, color),
                      },
                  },
                  {
                      .buffer = vbo.get(),
                      .attribute_trait {
                          .location = 2,
                          .data_type = ShaderDataType::Float2,
                          .offset = offsetof(VertexShaderInput, texture_coordinate),
                      },
                  },
                  {
                      .buffer = vbo.get(),
                      .attribute_trait {
                          .location = 3,
                          .data_type = ShaderDataType::Float,
                          .offset = offsetof(VertexShaderInput, texture),
                      },
                  } },
        sizeof(VertexShaderInput), ebo.get());
}

void Renderer2D::clear(std::uint32_t mask) {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.get_id());
    glClear(mask);
    if (!frame_buffer.is_swap_chain_target()) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Renderer2D::set_clear_color(float r, float g, float b, float a) { glClearColor(r, g, b, a); }

/*
 * TODO: deferred lighting + forward renderer
 * TODO: light sources, directional and ambient light
 * TODO: shadows?
 */
void Renderer2D::draw(std::span<const Renderer2D::Vertex> vertices, std::span<const std::uint32_t> indices,
    const glm::mat4& transform, std::uint32_t draw_mode) {
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
        flush();
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
        vertices_vec.push_back(VertexShaderInput { .position { vertex.position },
            .color { vertex.color },
            .texture_coordinate { vertex.texture_coordinate },
            .texture = float(texture_unit_map[vertex.texture]) });
    }

    std::ranges::transform(indices, std::back_inserter(indices_vec),
        [offset = vertices_vec.size() - vertices.size()](auto& i) { return i + std::uint32_t(offset); });
}

void Renderer2D::draw(const TransformComponent& transform, const SpriteComponent& sprite) {
    sprite_quads.push_back({
        .z = transform.translation.z,
        .transform = transform.get_matrix(),
        .vertices = build_sprite_quad(sprite),
    });
}

std::array<Renderer2D::Vertex, 4> Renderer2D::build_sprite_quad(const SpriteComponent& sprite) {
    return {
        Vertex {
            .position = { 1.0f, -1.0f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x + sprite.uv_rect.w, sprite.uv_rect.y },
            .texture = sprite.texture.id,
        },
        Vertex {
            .position = { -1.0f, 1.0f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x, sprite.uv_rect.y + sprite.uv_rect.h },
            .texture = sprite.texture.id,
        },
        Vertex {
            .position = { -1.0f, -1.0f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x, sprite.uv_rect.y },
            .texture = sprite.texture.id,
        },
        Vertex {
            .position = { 1.0f, 1.0f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x + sprite.uv_rect.w, sprite.uv_rect.y + sprite.uv_rect.h },
            .texture = sprite.texture.id,
        },
    };
}

void Renderer2D::draw(Scene& scene) {
    if (scene.registry.ctx().contains<ResourceManager&>()) {
        resources = &scene.registry.ctx().get<ResourceManager&>();
    }

    for (auto entity : scene.registry.view<k2::Camera, k2::MainCamera>()) {
        camera = scene.registry.get<k2::Camera>(entity);
        break;
    }

    scene.registry.view<k2::TransformComponent, k2::SpriteComponent>().each(
        [&](auto, const auto& transform, const auto& sprite) { draw(transform, sprite); });
}

void Renderer2D::render() {
    // Blending is order-dependent: sprites are drawn back to front.
    std::ranges::stable_sort(sprite_quads, {}, &SpriteQuad::z);
    const std::array<std::uint32_t, 6> indices { 0, 1, 2, 0, 3, 1 };
    for (const auto& quad : sprite_quads) {
        draw(quad.vertices, indices, quad.transform);
    }
    sprite_quads.clear();

    flush();
}

void Renderer2D::flush() {
    std::array<GLint, 4> saved_viewport {};
    if (!frame_buffer.is_swap_chain_target()) {
        glGetIntegerv(GL_VIEWPORT, saved_viewport.data());
        auto& traits = frame_buffer.get_traits();
        glViewport(0, 0, (GLsizei)traits.width, (GLsizei)traits.height);
    }

    // Depth testing breaks blended sprites: transparent fragments still write
    // depth, so anything behind them would be rejected. Ordering is the CPU
    // sort's job.
    auto depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    auto blend_was_enabled = glIsEnabled(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& [draw_mode, vertices] : vertices_buffer) {
        auto& indices = indices_buffer[draw_mode];

        std::vector<std::int32_t> tex_unit_vec(texture_unit_map.size());
        std::iota(tex_unit_vec.begin(), tex_unit_vec.end(), 0);

        if (resources != nullptr) {
            for (auto& [texture_id, texture_unit_index] : texture_unit_map) {
                auto* texture = resources->try_get<Texture2D>(texture_id);
                if (texture) {
                    texture->bind(texture_unit_index);
                }
            }
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

    if (depth_was_enabled) {
        glEnable(GL_DEPTH_TEST);
    }
    if (!blend_was_enabled) {
        glDisable(GL_BLEND);
    }

    if (!frame_buffer.is_swap_chain_target()) {
        glViewport(saved_viewport[0], saved_viewport[1], saved_viewport[2], saved_viewport[3]);
    }

    texture_unit_map.clear();
    for (auto& [draw_mode, vertices] : vertices_buffer) {
        vertices.clear();
    }
    for (auto& [draw_mode, indices] : indices_buffer) {
        indices.clear();
    }
}
}
