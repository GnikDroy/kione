#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "core/resource_container.hpp"
#include "core/utils.hpp"

namespace k2 {

struct Glyph {
    Rectf atlas_uv {};
    glm::vec2 size {};
    glm::vec2 bearing {};
    float advance {};
};

struct TextMetrics {
    std::vector<float> line_widths {};
    float width {};
    float height {};
};

struct Font {
    ResourceID atlas {};
    std::unordered_map<char, Glyph> glyphs {};
    float ascent {};
    float descent {};
    float line_gap {};
    float bake_px {};

    [[nodiscard]] TextMetrics measure(std::string_view text, float size) const;
};

struct BakedFont {
    std::vector<std::uint8_t> pixels {};
    int width {};
    int height {};
    std::unordered_map<char, Glyph> glyphs {};
    float ascent {};
    float descent {};
    float line_gap {};
    float bake_px {};

    BakedFont() = default;

    explicit BakedFont(std::span<const std::byte> ttf, float pixel_height = 48.0f);
};

}
