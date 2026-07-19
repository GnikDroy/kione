#pragma once

#include "core/resources.hpp"

#include "rendering/buffer.hpp"
#include "rendering/frame_buffer.hpp"
#include "rendering/vertex_array.hpp"

#include "components/camera.hpp"
#include "components/sprite.hpp"
#include "components/transform.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

namespace k2 {
struct Scene;
struct Font;
struct TextComponent;

class Renderer2D {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texture_coordinate;
        ResourceID texture;
    };
    struct VertexShaderInput {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texture_coordinate;
        float texture;
    };
    Camera camera;

private:
    struct SpriteQuad {
        float z;
        glm::mat4 transform;
        std::array<Vertex, 4> vertices;
        bool unlit;
    };

    struct PointLightDraw {
        glm::mat4 model;
        glm::vec3 color;
    };
    struct SpotLightDraw {
        glm::mat4 model;
        glm::vec3 color;
        float cos_inner;
        float cos_outer;
    };
    struct SpriteLightDraw {
        glm::mat4 model;
        glm::vec3 color;
        ResourceID texture;
    };

    Program default_shader;
    Program light_shader;
    Program composite_shader;
    Program text_shader;

    std::vector<SpriteQuad> sprite_quads {};
    std::vector<SpriteQuad> text_quads {};
    std::unordered_map<std::uint32_t, std::vector<Renderer2D::VertexShaderInput>> vertices_buffer {};
    std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> indices_buffer {};
    std::unordered_map<ResourceID, std::uint32_t> texture_unit_map {};

    std::size_t max_vertices = 100'000;
    // GL 4.1 guarantees only 16 fragment texture units;
    std::uint32_t max_textures = 16;
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ebo;
    VertexArray light_vao;
    VertexBuffer light_vbo;
    VertexArray empty_vao;
    FrameBuffer frame_buffer;
    FrameBuffer albedo_buffer;
    FrameBuffer light_buffer;
    const FrameBuffer* batch_target = &frame_buffer;
    Program* batch_shader = &default_shader;
    Texture2D fallback_texture = Texture2D::create_white_texture<uint8_t>();
    // Picked up from the scene's registry context in draw(Scene), or set explicitly
    // via set_resources() when drawing without a scene.
    ResourceManager* resources {};

    glm::vec4 clear_color { 0.0f, 0.0f, 0.0f, 1.0f };
    glm::vec3 ambient_light {};
    bool has_lights {};
    std::vector<PointLightDraw> point_lights {};
    std::vector<SpotLightDraw> spot_lights {};
    std::vector<SpriteLightDraw> sprite_lights {};

public:
    Renderer2D();

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    FrameBuffer& set_frame_buffer(FrameBuffer&& fb) { return frame_buffer = std::move(fb); }

    FrameBuffer& get_frame_buffer() { return frame_buffer; }

    void set_resources(ResourceManager& resource_manager) { resources = &resource_manager; }

    void clear(std::uint32_t mask = GL_COLOR_BUFFER_BIT);

    void set_clear_color(float r, float g, float b, float a);

    void draw(std::span<const Renderer2D::Vertex> vertices, std::span<const std::uint32_t> indices,
        const glm::mat4& transform = glm::mat4(1.0f), std::uint32_t draw_mode = GL_TRIANGLES);

    void draw(const TransformComponent& transform, const SpriteComponent& sprite);

    void draw(Scene& scene);

    void render();

private:
    static std::array<Vertex, 4> build_sprite_quad(const SpriteComponent& sprite);
    void layout_text(const TextComponent& text, const Font& font, const glm::mat4& world);
    void draw_text_pass();

    void collect_lights(Scene& scene);
    void ensure_light_targets(std::size_t width, std::size_t height);
    void light_pass();
    void composite_pass();
    void flush();
    void flush_batches(const FrameBuffer& target);
    void flush_batches(const FrameBuffer& target, Program& shader);
};
}
