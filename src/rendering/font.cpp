#include "rendering/font.hpp"

#include <algorithm>
#include <stdexcept>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace k2 {

TextMetrics Font::measure(std::string_view text, float size) const {
    float scale = bake_px > 0.0f ? size / bake_px : 0.0f;
    TextMetrics metrics { .line_widths = { 0.0f } };
    for (char c : text) {
        if (c == '\n') {
            metrics.line_widths.push_back(0.0f);
        } else if (auto it = glyphs.find(c); it != glyphs.end()) {
            metrics.line_widths.back() += it->second.advance * scale;
        }
    }
    float line_advance = (ascent - descent + line_gap) * scale;
    metrics.width = std::ranges::max(metrics.line_widths);
    metrics.height = (ascent - descent) * scale + float(metrics.line_widths.size() - 1) * line_advance;
    return metrics;
}

std::vector<GlyphQuad> Font::layout(std::string_view text, float size) const {
    float scale = bake_px > 0.0f ? size / bake_px : 0.0f;
    float line_advance = (ascent - descent + line_gap) * scale;
    auto metrics = measure(text, size);

    std::vector<GlyphQuad> quads;
    std::size_t line = 0;
    float pen_x = -metrics.line_widths[line] / 2.0f;
    float pen_y = metrics.height / 2.0f - ascent * scale;

    for (char c : text) {
        if (c == '\n') {
            line++;
            pen_x = -metrics.line_widths[line] / 2.0f;
            pen_y -= line_advance;
            continue;
        }
        auto it = glyphs.find(c);
        if (it == glyphs.end()) {
            continue;
        }
        const auto& glyph = it->second;
        if (glyph.size.x > 0.0f && glyph.size.y > 0.0f) {
            float left = pen_x + glyph.bearing.x * scale;
            float top = pen_y - glyph.bearing.y * scale;
            quads.push_back({
                .rect = { .x = left,
                    .y = top - glyph.size.y * scale,
                    .w = glyph.size.x * scale,
                    .h = glyph.size.y * scale },
                .uv = glyph.atlas_uv,
            });
        }
        pen_x += glyph.advance * scale;
    }
    return quads;
}

namespace {

    constexpr char first_char = 32;
    constexpr char last_char = 126;
    constexpr int padding = 5;
    constexpr unsigned char onedge = 128;
    constexpr float distance_per_pixel = float(onedge) / float(padding);
    constexpr int gutter = 1; // keeps neighbours from bleeding under bilinear sampling

    struct Baked {
        char code;
        std::vector<std::uint8_t> pixels;
        int w, h, xoff, yoff;
        float advance;
    };

}

BakedFont::BakedFont(std::span<const std::byte> ttf, float pixel_height) {
    const auto* data = reinterpret_cast<const unsigned char*>(ttf.data());

    if (ttf.size() < 4) {
        throw std::runtime_error("Font data too small to be a TTF/OTF");
    }
    std::uint32_t tag = std::uint32_t(data[0]) << 24 | std::uint32_t(data[1]) << 16 | std::uint32_t(data[2]) << 8
        | std::uint32_t(data[3]);
    if (tag != 0x00010000 && tag != 0x74727565 && tag != 0x74797031 && tag != 0x4F54544F && tag != 0x74746366) {
        throw std::runtime_error("Not a TTF/OTF font (bad sfnt tag)");
    }

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, data, stbtt_GetFontOffsetForIndex(data, 0))) {
        throw std::runtime_error("Failed to parse TTF font");
    }

    float scale = stbtt_ScaleForPixelHeight(&info, pixel_height);
    int raw_ascent, raw_descent, raw_line_gap;
    stbtt_GetFontVMetrics(&info, &raw_ascent, &raw_descent, &raw_line_gap);

    std::vector<Baked> baked;
    std::vector<stbrp_rect> rects;
    for (char code = first_char; code <= last_char; code++) {
        int advance_width, left_bearing;
        stbtt_GetCodepointHMetrics(&info, code, &advance_width, &left_bearing);

        int w = 0, h = 0, xoff = 0, yoff = 0;
        unsigned char* sdf
            = stbtt_GetCodepointSDF(&info, scale, code, padding, onedge, distance_per_pixel, &w, &h, &xoff, &yoff);

        Baked entry { .code = code, .w = w, .h = h, .xoff = xoff, .yoff = yoff, .advance = advance_width * scale };
        if (sdf != nullptr) {
            entry.pixels.assign(sdf, sdf + std::size_t(w) * std::size_t(h));
            stbtt_FreeSDF(sdf, nullptr);
            rects.push_back({ .id = int(baked.size()), .w = stbrp_coord(w + gutter), .h = stbrp_coord(h + gutter) });
        }
        baked.push_back(std::move(entry));
    }

    int atlas_size = 512;
    std::vector<stbrp_node> nodes;
    for (;;) {
        nodes.assign(std::size_t(atlas_size), {});
        stbrp_context ctx;
        stbrp_init_target(&ctx, atlas_size, atlas_size, nodes.data(), atlas_size);
        if (stbrp_pack_rects(&ctx, rects.data(), int(rects.size())) != 0) {
            break;
        }
        atlas_size *= 2;
        if (atlas_size > 4096) {
            throw std::runtime_error("SDF font atlas exceeded 4096px");
        }
    }

    width = atlas_size;
    height = atlas_size;
    pixels.assign(std::size_t(atlas_size) * std::size_t(atlas_size), 0);
    ascent = raw_ascent * scale;
    descent = raw_descent * scale;
    line_gap = raw_line_gap * scale;
    bake_px = pixel_height;

    for (const auto& rect : rects) {
        const auto& entry = baked[std::size_t(rect.id)];
        for (int row = 0; row < entry.h; row++) {
            for (int col = 0; col < entry.w; col++) {
                pixels[std::size_t(rect.y + row) * atlas_size + (rect.x + col)]
                    = entry.pixels[std::size_t(row) * entry.w + col];
            }
        }
    }

    auto fatlas = float(atlas_size);
    for (const auto& rect : rects) {
        const auto& entry = baked[std::size_t(rect.id)];
        glyphs[entry.code] = Glyph {
            .atlas_uv = { rect.x / fatlas, rect.y / fatlas, entry.w / fatlas, entry.h / fatlas },
            .size = { float(entry.w), float(entry.h) },
            .bearing = { float(entry.xoff), float(entry.yoff) },
            .advance = entry.advance,
        };
    }
    // Glyphs with no outline (space) still need an advance for layout.
    for (const auto& entry : baked) {
        if (!glyphs.contains(entry.code)) {
            glyphs[entry.code] = Glyph { .advance = entry.advance };
        }
    }
}

}
