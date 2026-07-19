#include "rendering/renderer2D.hpp"

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <unordered_set>

#include <glm/gtx/quaternion.hpp>

#include "components/light.hpp"
#include "components/text.hpp"
#include "core/logger.hpp"
#include "core/scene.hpp"
#include "rendering/font.hpp"

namespace k2 {

static Program load_program(const char* vertex, const char* fragment) {
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
    : default_shader { load_program("res/shaders/2D_vs.glsl", "res/shaders/2D_fs.glsl") }
    , light_shader { load_program("res/shaders/light_vs.glsl", "res/shaders/light_fs.glsl") }
    , composite_shader { load_program("res/shaders/composite_vs.glsl", "res/shaders/composite_fs.glsl") }
    , text_shader { load_program("res/shaders/text_vs.glsl", "res/shaders/text_fs.glsl") } {
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

    const std::array<glm::vec2, 6> quad { { { -1.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, -1.0f },
        { 1.0f, 1.0f }, { -1.0f, 1.0f } } };
    light_vbo = VertexBuffer { quad.size() * sizeof(glm::vec2) };
    light_vbo.set(quad.data(), quad.size() * sizeof(glm::vec2));
    light_vao.apply({ {
                        .buffer = light_vbo.get(),
                        .attribute_trait {
                            .location = 0,
                            .data_type = ShaderDataType::Float2,
                            .offset = 0,
                        },
                    } },
        sizeof(glm::vec2));
}

void Renderer2D::clear(std::uint32_t mask) {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.get_id());
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(mask);
    if (!frame_buffer.is_swap_chain_target()) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Renderer2D::set_clear_color(float r, float g, float b, float a) { clear_color = { r, g, b, a }; }

/*
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
        flush_batches(*batch_target, *batch_shader);
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
        .unlit = sprite.unlit,
    });
}

std::array<Renderer2D::Vertex, 4> Renderer2D::build_sprite_quad(const SpriteComponent& sprite) {
    return {
        Vertex {
            .position = { 0.5f, -0.5f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x + sprite.uv_rect.w, sprite.uv_rect.y },
            .texture = sprite.texture.id,
        },
        Vertex {
            .position = { -0.5f, 0.5f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x, sprite.uv_rect.y + sprite.uv_rect.h },
            .texture = sprite.texture.id,
        },
        Vertex {
            .position = { -0.5f, -0.5f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x, sprite.uv_rect.y },
            .texture = sprite.texture.id,
        },
        Vertex {
            .position = { 0.5f, 0.5f, 0.0f },
            .color = sprite.color,
            .texture_coordinate = { sprite.uv_rect.x + sprite.uv_rect.w, sprite.uv_rect.y + sprite.uv_rect.h },
            .texture = sprite.texture.id,
        },
    };
}

static glm::mat4 unscaled_world(entt::registry& registry, entt::entity entity) {
    auto world = TransformComponent::world(registry, entity);
    glm::mat4 basis { 1.0f };
    for (int column = 0; column < 3; column++) {
        auto axis = glm::vec3(world[column]);
        auto length = glm::length(axis);
        basis[column] = length > 0.0f ? glm::vec4(axis / length, 0.0f) : world[column];
    }
    basis[3] = world[3];
    return basis;
}

void Renderer2D::collect_lights(Scene& scene) {
    ambient_light = {};
    point_lights.clear();
    spot_lights.clear();
    sprite_lights.clear();
    has_lights = false;

    auto& registry = scene.registry;

    registry.view<AmbientLight>().each([&](auto, const AmbientLight& light) {
        ambient_light += light.color * light.intensity;
        has_lights = true;
    });

    registry.view<TransformComponent, PointLight>().each([&](auto entity, const auto&, const PointLight& light) {
        auto model
            = unscaled_world(registry, entity) * glm::scale(glm::mat4(1.0f), { light.radius, light.radius, 1.0f });
        point_lights.push_back({ model, light.color * light.intensity });
        has_lights = true;
    });

    registry.view<TransformComponent, SpotLight>().each([&](auto entity, const auto&, const SpotLight& light) {
        auto model
            = unscaled_world(registry, entity) * glm::scale(glm::mat4(1.0f), { light.radius, light.radius, 1.0f });
        spot_lights.push_back({ model, light.color * light.intensity,
            std::cos(std::min(light.inner_angle, light.outer_angle)), std::cos(light.outer_angle) });
        has_lights = true;
    });

    registry.view<TransformComponent, SpriteLight>().each([&](auto entity, const auto&, const SpriteLight& light) {
        auto model = TransformComponent::world(registry, entity) * glm::scale(glm::mat4(1.0f), { 0.5f, 0.5f, 1.0f });
        sprite_lights.push_back({ model, light.color * light.intensity, light.texture.id });
        has_lights = true;
    });
}

void Renderer2D::draw(Scene& scene) {
    if (scene.registry.ctx().contains<ResourceManager&>()) {
        resources = &scene.registry.ctx().get<ResourceManager&>();
    }

    collect_lights(scene);

    scene.registry.view<k2::TransformComponent, k2::SpriteComponent>().each(
        [&](auto entity, const auto&, const auto& sprite) {
            auto world = TransformComponent::world(scene.registry, entity);
            sprite_quads.push_back({
                .z = world[3][2],
                .transform = world,
                .vertices = build_sprite_quad(sprite),
                .unlit = sprite.unlit,
            });
        });

    scene.registry.view<k2::TransformComponent, k2::TextComponent>().each(
        [&](auto entity, const auto&, const TextComponent& text) {
            const auto* font = resources != nullptr ? resources->try_get<Font>(text.font.id) : nullptr;
            if (font == nullptr) {
                return;
            }
            layout_text(text, *font, unscaled_world(scene.registry, entity));
        });
}

void Renderer2D::layout_text(const TextComponent& text, const Font& font, const glm::mat4& world) {
    float scale = font.bake_px > 0.0f ? text.size / font.bake_px : 0.0f;
    float line_advance = (font.ascent - font.descent + font.line_gap) * scale;

    // The block is centered on the origin
    auto metrics = font.measure(text.text, text.size);
    std::size_t line = 0;
    float pen_x = -metrics.line_widths[line] / 2.0f;
    float pen_y = metrics.height / 2.0f - font.ascent * scale;

    for (char c : text.text) {
        if (c == '\n') {
            line++;
            pen_x = -metrics.line_widths[line] / 2.0f;
            pen_y -= line_advance;
            continue;
        }
        auto it = font.glyphs.find(c);
        if (it == font.glyphs.end()) {
            continue;
        }
        const auto& glyph = it->second;
        if (glyph.size.x > 0.0f && glyph.size.y > 0.0f) {
            float left = pen_x + glyph.bearing.x * scale;
            float right = left + glyph.size.x * scale;
            float top = pen_y - glyph.bearing.y * scale;
            float bottom = top - glyph.size.y * scale;
            const auto& uv = glyph.atlas_uv;

            std::array<Vertex, 4> vertices { {
                { .position = { right, top, 0.0f },
                    .color = text.color,
                    .texture_coordinate = { uv.x + uv.w, uv.y },
                    .texture = font.atlas },
                { .position = { left, bottom, 0.0f },
                    .color = text.color,
                    .texture_coordinate = { uv.x, uv.y + uv.h },
                    .texture = font.atlas },
                { .position = { left, top, 0.0f },
                    .color = text.color,
                    .texture_coordinate = { uv.x, uv.y },
                    .texture = font.atlas },
                { .position = { right, bottom, 0.0f },
                    .color = text.color,
                    .texture_coordinate = { uv.x + uv.w, uv.y + uv.h },
                    .texture = font.atlas },
            } };
            text_quads.push_back({ .z = world[3][2], .transform = world, .vertices = vertices, .unlit = true });
        }
        pen_x += glyph.advance * scale;
    }
}

void Renderer2D::ensure_light_targets(std::size_t width, std::size_t height) {
    if (albedo_buffer.get_traits().width == width && albedo_buffer.get_traits().height == height) {
        return;
    }
    FrameBuffer::Traits traits { .width = width,
        .height = height,
        .attachments { {
            .buffer_type = FrameBuffer::Attachment::BufferType::Texture,
            .type = FrameBuffer::Attachment::Type::Color,
        } } };
    albedo_buffer = FrameBuffer { traits };
    light_buffer = FrameBuffer { traits };
}

void Renderer2D::light_pass() {
    auto& traits = light_buffer.get_traits();
    glBindFramebuffer(GL_FRAMEBUFFER, light_buffer.get_id());
    glViewport(0, 0, (GLsizei)traits.width, (GLsizei)traits.height);
    glClearColor(ambient_light.r, ambient_light.g, ambient_light.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    light_shader.use().set_uniform("view", camera.get_view()).set_uniform("projection", camera.get_projection());
    light_vao.bind();

    for (const auto& light : point_lights) {
        light_shader.set_uniform("mode", 0).set_uniform("model", light.model).set_uniform("color", light.color);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    for (const auto& light : spot_lights) {
        light_shader.set_uniform("mode", 1)
            .set_uniform("model", light.model)
            .set_uniform("color", light.color)
            .set_uniform("cos_inner", light.cos_inner)
            .set_uniform("cos_outer", light.cos_outer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    for (const auto& light : sprite_lights) {
        auto* texture = resources != nullptr ? resources->try_get<Texture2D>(light.texture) : nullptr;
        (texture != nullptr ? *texture : fallback_texture).bind(0);
        light_shader.set_uniform("mode", 2)
            .set_uniform("model", light.model)
            .set_uniform("color", light.color)
            .set_uniform("light_texture", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    VertexArray::unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer2D::composite_pass() {
    std::array<GLint, 4> saved_viewport {};
    glGetIntegerv(GL_VIEWPORT, saved_viewport.data());
    if (!frame_buffer.is_swap_chain_target()) {
        auto& traits = frame_buffer.get_traits();
        glViewport(0, 0, (GLsizei)traits.width, (GLsizei)traits.height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.get_id());
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedo_buffer.get_traits().attachments.front().id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, light_buffer.get_traits().attachments.front().id);

    composite_shader.use().set_uniform("albedo_texture", 0).set_uniform("light_texture", 1);

    empty_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    VertexArray::unbind();

    if (!frame_buffer.is_swap_chain_target()) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glViewport(saved_viewport[0], saved_viewport[1], saved_viewport[2], saved_viewport[3]);
}

void Renderer2D::render() {
    // Blending is order-dependent: sprites are drawn back to front.
    std::ranges::stable_sort(sprite_quads, {}, &SpriteQuad::z);
    const std::array<std::uint32_t, 6> indices { 0, 1, 2, 0, 3, 1 };

    if (!has_lights) {
        for (const auto& quad : sprite_quads) {
            draw(quad.vertices, indices, quad.transform);
        }
        sprite_quads.clear();
        flush_batches(frame_buffer);
        draw_text_pass();
        return;
    }

    std::size_t width, height;
    if (frame_buffer.is_swap_chain_target()) {
        std::array<GLint, 4> viewport {};
        glGetIntegerv(GL_VIEWPORT, viewport.data());
        width = (std::size_t)viewport[2];
        height = (std::size_t)viewport[3];
    } else {
        width = frame_buffer.get_traits().width;
        height = frame_buffer.get_traits().height;
    }
    ensure_light_targets(std::max<std::size_t>(width, 1), std::max<std::size_t>(height, 1));

    glBindFramebuffer(GL_FRAMEBUFFER, albedo_buffer.get_id());
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    batch_target = &albedo_buffer;
    bool any_unlit = false;
    for (const auto& quad : sprite_quads) {
        if (quad.unlit) {
            any_unlit = true;
            continue;
        }
        draw(quad.vertices, indices, quad.transform);
    }
    flush_batches(albedo_buffer);
    batch_target = &frame_buffer;

    light_pass();
    composite_pass();

    // Drawn after the composite so they skip the light multiply.
    // Unlit sprites always sit on top of lit sprites regardless of z.
    if (any_unlit) {
        for (const auto& quad : sprite_quads) {
            if (quad.unlit) {
                draw(quad.vertices, indices, quad.transform);
            }
        }
        flush_batches(frame_buffer);
    }
    sprite_quads.clear();

    draw_text_pass();
    has_lights = false;
}

void Renderer2D::draw_text_pass() {
    if (text_quads.empty()) {
        return;
    }
    const std::array<std::uint32_t, 6> indices { 0, 1, 2, 0, 3, 1 };
    std::ranges::stable_sort(text_quads, {}, &SpriteQuad::z);
    batch_target = &frame_buffer;
    batch_shader = &text_shader;
    for (const auto& quad : text_quads) {
        draw(quad.vertices, indices, quad.transform);
    }
    flush_batches(frame_buffer, text_shader);
    batch_shader = &default_shader;
    text_quads.clear();
}

void Renderer2D::flush() { flush_batches(*batch_target, *batch_shader); }

void Renderer2D::flush_batches(const FrameBuffer& target) { flush_batches(target, default_shader); }

void Renderer2D::flush_batches(const FrameBuffer& target, Program& shader) {
    std::array<GLint, 4> saved_viewport {};
    if (!target.is_swap_chain_target()) {
        glGetIntegerv(GL_VIEWPORT, saved_viewport.data());
        auto& traits = target.get_traits();
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
        std::ranges::iota(tex_unit_vec, 0);

        // Units always get a binding: skipping missing textures would leave stale
        // bindings from earlier flushes visible.
        for (auto& [texture_id, texture_unit_index] : texture_unit_map) {
            auto* texture = resources != nullptr ? resources->try_get<Texture2D>(texture_id) : nullptr;
            (texture != nullptr ? *texture : fallback_texture).bind(texture_unit_index);
        }

        shader.use()
            .set_uniform("texture_list", std::span { tex_unit_vec.data(), tex_unit_vec.size() })
            .set_uniform("model", glm::mat4(1.0f))
            .set_uniform("view", camera.get_view())
            .set_uniform("projection", camera.get_projection());

        vao.bind();
        vbo.set(vertices.data(), sizeof(vertices[0]) * vertices.size());
        ebo.set(indices.data(), sizeof(indices[0]) * indices.size());
        glBindFramebuffer(GL_FRAMEBUFFER, target.get_id());
        glDrawElements(draw_mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        if (!target.is_swap_chain_target()) {
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

    if (!target.is_swap_chain_target()) {
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
