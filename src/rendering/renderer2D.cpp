#include "rendering/renderer2D.hpp"

#include <algorithm>

#include <glm/gtx/quaternion.hpp>

#include "components/light.hpp"
#include "components/text.hpp"
#include "components/tilemap.hpp"
#include "core/logger.hpp"
#include "core/scene.hpp"
#include "rendering/embedded_shaders.hpp"
#include "rendering/font.hpp"

namespace k2 {

constexpr std::array<std::uint32_t, 6> quad_indices { 0, 1, 2, 0, 3, 1 };

static Program load_program(std::string_view vertex, std::string_view fragment) {
    auto vertex_shader = k2::Shader(GL_VERTEX_SHADER, std::string { vertex });
    if (!vertex_shader) {
        k2::Log::core().critical(vertex_shader.error_msg().value());
    }

    auto fragment_shader = k2::Shader(GL_FRAGMENT_SHADER, std::string { fragment });
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
    : default_shader { load_program(embedded_shaders::batch_vs, embedded_shaders::batch_fs) }
    , light_shader { load_program(embedded_shaders::light_vs, embedded_shaders::light_fs) }
    , composite_shader { load_program(embedded_shaders::composite_vs, embedded_shaders::composite_fs) }
    , text_shader { load_program(embedded_shaders::text_vs, embedded_shaders::text_fs) }
    , bright_shader { load_program(embedded_shaders::composite_vs, embedded_shaders::bloom_bright_fs) }
    , downsample_shader { load_program(embedded_shaders::composite_vs, embedded_shaders::bloom_downsample_fs) }
    , upsample_shader { load_program(embedded_shaders::composite_vs, embedded_shaders::bloom_upsample_fs) }
    , present_shader { load_program(embedded_shaders::composite_vs, embedded_shaders::present_fs) } {
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

std::array<Vertex2D, 4> Renderer2D::build_sprite_quad(const SpriteComponent& sprite, const Rectf& uv) {
    auto color = glm::vec4 { glm::vec3 { sprite.color } * sprite.intensity, sprite.color.a };
    auto half = sprite.size * 0.5f;
    return {
        Vertex2D {
            .position = { half.x, -half.y, 0.0f },
            .color = color,
            .texture_coordinate = { uv.x + uv.w, uv.y },
            .texture = sprite.texture.id,
        },
        Vertex2D {
            .position = { -half.x, half.y, 0.0f },
            .color = color,
            .texture_coordinate = { uv.x, uv.y + uv.h },
            .texture = sprite.texture.id,
        },
        Vertex2D {
            .position = { -half.x, -half.y, 0.0f },
            .color = color,
            .texture_coordinate = { uv.x, uv.y },
            .texture = sprite.texture.id,
        },
        Vertex2D {
            .position = { half.x, half.y, 0.0f },
            .color = color,
            .texture_coordinate = { uv.x + uv.w, uv.y + uv.h },
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

void Renderer2D::push_primitive(shapes::Mesh mesh, const PrimitiveStyle& style) {
    if (mesh.positions.empty()) {
        return;
    }
    Drawable drawable { .z = style.z, .unlit = style.unlit };
    drawable.vertices.reserve(mesh.positions.size());
    for (auto position : mesh.positions) {
        drawable.vertices.push_back(Vertex2D { .position = { position.x, position.y, style.z },
            .color = style.color,
            .texture_coordinate = { 0.0f, 0.0f },
            .texture = ResourceID {} });
    }
    drawable.indices = std::move(mesh.indices);
    drawables.push_back(std::move(drawable));
}

void Renderer2D::draw_line(glm::vec2 a, glm::vec2 b, float width, const PrimitiveStyle& style) {
    push_primitive(shapes::line_mesh(a, b, width), style);
}

void Renderer2D::draw_rect(glm::vec2 center, glm::vec2 size, const PrimitiveStyle& style) {
    push_primitive(shapes::rect_mesh(center, size), style);
}

void Renderer2D::draw_rect_outline(glm::vec2 center, glm::vec2 size, float thickness, const PrimitiveStyle& style) {
    push_primitive(shapes::rect_outline_mesh(center, size, thickness), style);
}

void Renderer2D::draw_circle(glm::vec2 center, float radius, const PrimitiveStyle& style, int segments) {
    if (segments <= 0) {
        segments = shapes::circle_segment_count(radius);
    }
    push_primitive(shapes::circle_mesh(center, radius, segments), style);
}

void Renderer2D::draw_circle_outline(
    glm::vec2 center, float radius, float thickness, const PrimitiveStyle& style, int segments) {
    if (segments <= 0) {
        segments = shapes::circle_segment_count(radius);
    }
    push_primitive(shapes::circle_outline_mesh(center, radius, thickness, segments), style);
}

void Renderer2D::draw_point(glm::vec2 position, float size, const PrimitiveStyle& style) {
    push_primitive(shapes::rect_mesh(position, { size, size }), style);
}

void Renderer2D::draw_polygon(std::span<const glm::vec2> points, const PrimitiveStyle& style) {
    push_primitive(shapes::polygon_mesh(points), style);
}

void Renderer2D::draw_polyline(
    std::span<const glm::vec2> points, float width, bool closed, const PrimitiveStyle& style) {
    push_primitive(shapes::polyline_mesh(points, width, closed), style);
}

void Renderer2D::draw_command(const DrawCommand& command) {
    PrimitiveStyle style { .color = command.color, .z = command.z, .unlit = command.unlit };
    switch (command.kind) {
    case DrawCommand::Kind::Line: draw_line(command.a, command.b, command.width, style); break;
    case DrawCommand::Kind::Rect:
        command.filled ? draw_rect(command.a, command.b, style)
                       : draw_rect_outline(command.a, command.b, command.width, style);
        break;
    case DrawCommand::Kind::Circle:
        command.filled ? draw_circle(command.a, command.radius, style, command.segments)
                       : draw_circle_outline(command.a, command.radius, command.width, style, command.segments);
        break;
    case DrawCommand::Kind::Point: draw_point(command.a, command.width, style); break;
    case DrawCommand::Kind::Polygon:
        command.filled ? draw_polygon(command.points, style)
                       : draw_polyline(command.points, command.width, command.closed, style);
        break;
    }
}

void Renderer2D::collect_lights(Scene& scene) {
    ambient_light = {};
    point_lights.clear();
    spot_lights.clear();
    sprite_lights.clear();
    has_lights = false;

    auto& registry = scene.registry;

    bool has_environment = false;
    environment = {};
    registry.view<Environment>().each([&](auto, const Environment& env) {
        environment = env;
        has_environment = true;
    });

    ambient_light = environment.ambient_color * environment.ambient_intensity;
    has_lights = has_environment && ambient_light != glm::vec3 { 0.0f };
    if (!has_environment) {
        environment.clear_color = clear_color;
    }

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

    if (auto* draw_list = scene.registry.ctx().find<DrawList>()) {
        for (const auto& command : draw_list->commands) {
            draw_command(command);
        }
        draw_list->commands.clear();
        draw_list->overflowed = false;
    }

    scene.registry.view<k2::TransformComponent, k2::SpriteComponent>().each(
        [&](auto entity, const auto&, const auto& sprite) {
            auto world = TransformComponent::world(scene.registry, entity);

            // Region is authored in pixels; normalize to UV
            Rectf uv { .x = 0.0f, .y = 0.0f, .w = 1.0f, .h = 1.0f };
            const auto* texture = resources != nullptr ? resources->try_get<Texture2D>(sprite.texture.id) : nullptr;
            if (texture != nullptr && texture->width > 0 && texture->height > 0) {
                uv = { .x = sprite.region.x / float(texture->width),
                    .y = sprite.region.y / float(texture->height),
                    .w = sprite.region.w / float(texture->width),
                    .h = sprite.region.h / float(texture->height) };
            }

            auto quad = build_sprite_quad(sprite, uv);
            drawables.push_back(Drawable { .z = world[3][2],
                .transform = world,
                .vertices = { quad.begin(), quad.end() },
                .indices = { quad_indices.begin(), quad_indices.end() },
                .unlit = sprite.unlit,
                .blend = sprite.blend });
        });

    scene.registry.view<k2::TransformComponent, k2::TileMapComponent>().each(
        [&](auto entity, const auto&, const TileMapComponent& tilemap) {
            const auto* tileset = resources != nullptr ? resources->try_get<TileSet>(tilemap.tileset.id) : nullptr;
            if (tileset == nullptr || tileset->texture_size.x <= 0 || tileset->texture_size.y <= 0) {
                return;
            }
            collect_tilemap(tilemap, *tileset, TransformComponent::world(scene.registry, entity));
        });

    scene.registry.view<k2::TransformComponent, k2::TextComponent>().each(
        [&](auto entity, const auto&, const TextComponent& text) {
            const auto* font = resources != nullptr ? resources->try_get<Font>(text.font.id) : nullptr;
            if (font == nullptr) {
                return;
            }
            collect_text(text, *font, unscaled_world(scene.registry, entity));
        });
}

void Renderer2D::collect_tilemap(const TileMapComponent& tilemap, const TileSet& tileset, const glm::mat4& world) {
    Drawable drawable { .z = world[3][2], .transform = world, .unlit = tilemap.unlit };
    std::uint32_t quads = 0;
    for (int row = 0; row < tilemap.size.y; row++) {
        for (int col = 0; col < tilemap.size.x; col++) {
            auto index = tilemap[col, row];
            if (index == TileMapComponent::empty_tile || !tileset.contains(index)) {
                continue;
            }
            auto uv = tileset[index].uv;
            float left = float(col) * tilemap.tile_size.x;
            float right = left + tilemap.tile_size.x;
            float top = -float(row) * tilemap.tile_size.y;
            float bottom = top - tilemap.tile_size.y;
            drawable.vertices.push_back({ .position = { right, bottom, 0.0f },
                .color = tilemap.color,
                .texture_coordinate = { uv.x + uv.w, uv.y },
                .texture = tileset.texture.id });
            drawable.vertices.push_back({ .position = { left, top, 0.0f },
                .color = tilemap.color,
                .texture_coordinate = { uv.x, uv.y + uv.h },
                .texture = tileset.texture.id });
            drawable.vertices.push_back({ .position = { left, bottom, 0.0f },
                .color = tilemap.color,
                .texture_coordinate = { uv.x, uv.y },
                .texture = tileset.texture.id });
            drawable.vertices.push_back({ .position = { right, top, 0.0f },
                .color = tilemap.color,
                .texture_coordinate = { uv.x + uv.w, uv.y + uv.h },
                .texture = tileset.texture.id });
            for (auto corner : quad_indices) {
                drawable.indices.push_back(quads * 4 + corner);
            }
            quads++;
        }
    }
    if (quads > 0) {
        drawables.push_back(std::move(drawable));
    }
}

void Renderer2D::collect_text(const TextComponent& text, const Font& font, const glm::mat4& world) {
    for (const auto& quad : font.layout(text.text, text.size)) {
        float left = quad.rect.x;
        float right = quad.rect.x + quad.rect.w;
        float bottom = quad.rect.y;
        float top = quad.rect.y + quad.rect.h;
        const auto& uv = quad.uv;

        text_drawables.push_back(Drawable { .z = world[3][2],
            .transform = world,
            .vertices = { { .position = { right, top, 0.0f },
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
                    .texture = font.atlas } },
            .indices = { quad_indices.begin(), quad_indices.end() },
            .unlit = true });
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

    glDisable(GL_DEPTH_TEST);
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
}

void Renderer2D::composite_pass() {
    auto& traits = scene_buffer.get_traits();
    glBindFramebuffer(GL_FRAMEBUFFER, scene_buffer.get_id());
    glViewport(0, 0, GLsizei(traits.width), GLsizei(traits.height));
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedo_buffer.get_traits().attachments.front().id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, light_buffer.get_traits().attachments.front().id);

    composite_shader.use().set_uniform("albedo_texture", 0).set_uniform("light_texture", 1);

    empty_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    VertexArray::unbind();
}

void Renderer2D::ensure_scene_targets(std::size_t width, std::size_t height) {
    if (scene_buffer.get_traits().width == width && scene_buffer.get_traits().height == height) {
        return;
    }
    auto make_target = [](std::size_t target_width, std::size_t target_height) {
        FrameBuffer target { FrameBuffer::Traits { .width = target_width,
            .height = target_height,
            .attachments { {
                .buffer_type = FrameBuffer::Attachment::BufferType::Texture,
                .type = FrameBuffer::Attachment::Type::Color,
            } } } };
        // wrapping would bleed bloom across screen
        glBindTexture(GL_TEXTURE_2D, target.get_traits().attachments.front().id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        return target;
    };
    scene_buffer = make_target(width, height);
    bloom_mips.clear();
    auto mip_width = width / 2;
    auto mip_height = height / 2;
    while (bloom_mips.size() < 6 && mip_width >= 8 && mip_height >= 8) {
        bloom_mips.push_back(make_target(mip_width, mip_height));
        mip_width /= 2;
        mip_height /= 2;
    }
}

void Renderer2D::bloom_pass() {
    auto fullscreen_into = [&](FrameBuffer& target) {
        auto& traits = target.get_traits();
        glBindFramebuffer(GL_FRAMEBUFFER, target.get_id());
        glViewport(0, 0, GLsizei(traits.width), GLsizei(traits.height));
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };
    auto texel_of = [](const FrameBuffer& source) {
        return glm::vec2 { 1.0f / float(source.get_traits().width), 1.0f / float(source.get_traits().height) };
    };
    auto bind_source = [](const FrameBuffer& source) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source.get_traits().attachments.front().id);
    };

    empty_vao.bind();
    glDisable(GL_BLEND);

    bind_source(scene_buffer);
    bright_shader.use().set_uniform("scene_texture", 0).set_uniform("threshold", environment.bloom_threshold);
    fullscreen_into(bloom_mips.front());

    downsample_shader.use().set_uniform("source_texture", 0);
    for (std::size_t i = 1; i < bloom_mips.size(); i++) {
        bind_source(bloom_mips[i - 1]);
        downsample_shader.set_uniform("texel", texel_of(bloom_mips[i - 1]));
        fullscreen_into(bloom_mips[i]);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    upsample_shader.use().set_uniform("source_texture", 0);
    for (auto i = std::ptrdiff_t(bloom_mips.size()) - 2; i >= 0; i--) {
        bind_source(bloom_mips[std::size_t(i) + 1]);
        upsample_shader.set_uniform("texel", texel_of(bloom_mips[std::size_t(i) + 1]));
        fullscreen_into(bloom_mips[std::size_t(i)]);
    }
    glDisable(GL_BLEND);

    VertexArray::unbind();
}

void Renderer2D::present_pass(const std::array<GLint, 4>& viewport, bool bloom) {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer.get_id());
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_buffer.get_traits().attachments.front().id);
    glActiveTexture(GL_TEXTURE1);
    if (bloom) {
        glBindTexture(GL_TEXTURE_2D, bloom_mips.front().get_traits().attachments.front().id);
    } else {
        fallback_texture.bind(1);
    }

    present_shader.use()
        .set_uniform("scene_texture", 0)
        .set_uniform("bloom_texture", 1)
        .set_uniform("bloom_intensity", bloom ? environment.bloom_intensity : 0.0f);

    empty_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    VertexArray::unbind();
}

void Renderer2D::render() {
    std::ranges::stable_sort(drawables, {}, &Drawable::z);
    std::ranges::stable_sort(text_drawables, {}, &Drawable::z);

    std::array<GLint, 4> window_viewport {};
    glGetIntegerv(GL_VIEWPORT, window_viewport.data());

    auto viewport_of = [&](const FrameBuffer& target) -> std::array<GLint, 4> {
        if (target.is_swap_chain_target()) {
            return window_viewport;
        }
        const auto& traits = target.get_traits();
        return { 0, 0, GLint(traits.width), GLint(traits.height) };
    };
    auto pass_for = [&](const FrameBuffer& target, Program& shader, std::uint32_t blend_dst) {
        return Batcher2D::Pass { .target = &target,
            .shader = &shader,
            .blend_dst = blend_dst,
            .viewport = viewport_of(target),
            .camera = &camera,
            .resources = resources };
    };
    auto submit_pass = [&](const Batcher2D::Pass& pass, std::span<const Drawable> items, auto filter) {
        if (std::ranges::none_of(items, filter)) {
            return;
        }
        batcher.begin(pass);
        for (const auto& drawable : items) {
            if (filter(drawable)) {
                batcher.submit(drawable.vertices, drawable.indices, drawable.transform);
            }
        }
        batcher.end();
    };
    auto is_alpha = [](const Drawable& drawable) { return drawable.blend == BlendMode::Alpha; };
    auto is_additive = [](const Drawable& drawable) { return drawable.blend == BlendMode::Additive; };

    auto output_viewport = viewport_of(frame_buffer);
    auto scene_width = std::max<std::size_t>(std::size_t(output_viewport[2]), 1);
    auto scene_height = std::max<std::size_t>(std::size_t(output_viewport[3]), 1);
    ensure_scene_targets(scene_width, scene_height);

    if (has_lights) {
        ensure_light_targets(scene_width, scene_height);

        glBindFramebuffer(GL_FRAMEBUFFER, albedo_buffer.get_id());
        glClearColor(
            environment.clear_color.r, environment.clear_color.g, environment.clear_color.b, environment.clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        submit_pass(pass_for(albedo_buffer, default_shader, GL_ONE_MINUS_SRC_ALPHA), drawables,
            [&](const Drawable& drawable) { return !drawable.unlit && is_alpha(drawable); });
        light_pass();
        composite_pass();

        // Drawn after the composite so they skip the light multiply.
        // Unlit sits above lit regardless of z.
        submit_pass(pass_for(scene_buffer, default_shader, GL_ONE_MINUS_SRC_ALPHA), drawables,
            [&](const Drawable& drawable) { return drawable.unlit && is_alpha(drawable); });
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, scene_buffer.get_id());
        glClearColor(
            environment.clear_color.r, environment.clear_color.g, environment.clear_color.b, environment.clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        submit_pass(pass_for(scene_buffer, default_shader, GL_ONE_MINUS_SRC_ALPHA), drawables, is_alpha);
    }
    // Emitters are never lit; additive is order-independent within itself.
    submit_pass(pass_for(scene_buffer, default_shader, GL_ONE), drawables, is_additive);

    bool bloom = environment.bloom && environment.bloom_intensity > 0.0f && !bloom_mips.empty();
    if (bloom) {
        bloom_pass();
    }
    present_pass(output_viewport, bloom);

    submit_pass(pass_for(frame_buffer, text_shader, GL_ONE_MINUS_SRC_ALPHA), text_drawables,
        [](const Drawable&) { return true; });

    drawables.clear();
    text_drawables.clear();
    has_lights = false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(window_viewport[0], window_viewport[1], window_viewport[2], window_viewport[3]);
}

}
