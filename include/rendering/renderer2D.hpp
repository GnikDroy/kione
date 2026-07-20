#pragma once

#include "core/resources.hpp"

#include "rendering/batcher2D.hpp"
#include "rendering/buffer.hpp"
#include "rendering/draw_list.hpp"
#include "rendering/drawable.hpp"
#include "rendering/frame_buffer.hpp"
#include "rendering/shapes.hpp"
#include "rendering/vertex_array.hpp"

#include "components/camera.hpp"
#include "components/environment.hpp"
#include "components/sprite.hpp"
#include "components/transform.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace k2 {
struct Scene;
struct Font;
struct TextComponent;

struct PrimitiveStyle {
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
    float z { 0.0f };
    bool unlit { true };
};

class Renderer2D {
public:
    Camera camera;

private:
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
    Program bright_shader;
    Program downsample_shader;
    Program upsample_shader;
    Program present_shader;

    Batcher2D batcher {};
    std::vector<Drawable> drawables {};
    std::vector<Drawable> text_drawables {};

    VertexArray light_vao;
    VertexBuffer light_vbo;
    VertexArray empty_vao;
    FrameBuffer frame_buffer;
    FrameBuffer albedo_buffer;
    FrameBuffer light_buffer;
    FrameBuffer scene_buffer;
    std::vector<FrameBuffer> bloom_mips;
    Texture2D fallback_texture = Texture2D::create_white_texture<uint8_t>();
    ResourceManager* resources {};

    glm::vec4 clear_color { 0.0f, 0.0f, 0.0f, 1.0f };
    glm::vec3 ambient_light {};
    Environment environment {};
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

    void clear(std::uint32_t mask = GL_COLOR_BUFFER_BIT);

    void set_clear_color(float r, float g, float b, float a);

    void draw(Scene& scene);

    void render();

    void draw_line(glm::vec2 a, glm::vec2 b, float width, const PrimitiveStyle& style = {});
    void draw_rect(glm::vec2 center, glm::vec2 size, const PrimitiveStyle& style = {});
    void draw_rect_outline(glm::vec2 center, glm::vec2 size, float thickness, const PrimitiveStyle& style = {});
    void draw_circle(glm::vec2 center, float radius, const PrimitiveStyle& style = {}, int segments = 0);
    void draw_circle_outline(
        glm::vec2 center, float radius, float thickness, const PrimitiveStyle& style = {}, int segments = 0);
    void draw_point(glm::vec2 position, float size, const PrimitiveStyle& style = {});
    void draw_polygon(std::span<const glm::vec2> points, const PrimitiveStyle& style = {});
    void draw_polyline(std::span<const glm::vec2> points, float width, bool closed, const PrimitiveStyle& style = {});

private:
    void push_primitive(shapes::Mesh mesh, const PrimitiveStyle& style);
    void draw_command(const DrawCommand& command);

    static std::array<Vertex2D, 4> build_sprite_quad(const SpriteComponent& sprite);
    void collect_text(const TextComponent& text, const Font& font, const glm::mat4& world);

    void collect_lights(Scene& scene);
    void ensure_light_targets(std::size_t width, std::size_t height);
    void ensure_scene_targets(std::size_t width, std::size_t height);
    void light_pass();
    void composite_pass();
    void bloom_pass();
    void present_pass(const std::array<GLint, 4>& viewport, bool bloom);
};
}
