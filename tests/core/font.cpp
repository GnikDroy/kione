#include <catch2/catch_all.hpp>

#include "rendering/font.hpp"

#include <cstddef>
#include <fstream>
#include <iterator>
#include <vector>

namespace {
std::vector<std::byte> read_file(const char* path) {
    std::ifstream in { path, std::ios::binary };
    std::vector<char> raw { std::istreambuf_iterator<char> { in }, std::istreambuf_iterator<char> {} };
    std::vector<std::byte> bytes(raw.size());
    for (std::size_t i = 0; i < raw.size(); i++) {
        bytes[i] = std::byte(raw[i]);
    }
    return bytes;
}
}

TEST_CASE("BakedFont produces an atlas and glyph metrics") {
    auto ttf = read_file(K2_TEST_FONT);
    REQUIRE_FALSE(ttf.empty());

    auto font = k2::BakedFont { ttf, 48.0f };

    REQUIRE(font.width > 0);
    REQUIRE(font.height > 0);
    REQUIRE(font.pixels.size() == std::size_t(font.width) * std::size_t(font.height));
    REQUIRE(font.bake_px == 48.0f);
    REQUIRE(font.ascent > 0.0f);
    REQUIRE(font.descent < 0.0f);

    // Printable ASCII is all present.
    REQUIRE(font.glyphs.contains('A'));
    REQUIRE(font.glyphs.contains('z'));
    REQUIRE(font.glyphs.contains('0'));
    REQUIRE(font.glyphs.contains(' '));

    const auto& a = font.glyphs.at('A');
    REQUIRE(a.advance > 0.0f);
    REQUIRE(a.size.x > 0.0f); // 'A' has an outline
    REQUIRE(a.size.y > 0.0f);
    // uv rect is normalized inside the atlas
    REQUIRE(a.atlas_uv.x >= 0.0f);
    REQUIRE(a.atlas_uv.y >= 0.0f);
    REQUIRE(a.atlas_uv.x + a.atlas_uv.w <= 1.0f);
    REQUIRE(a.atlas_uv.y + a.atlas_uv.h <= 1.0f);
}

TEST_CASE("BakedFont gives space an advance but no outline") {
    auto ttf = read_file(K2_TEST_FONT);
    auto font = k2::BakedFont { ttf, 48.0f };

    const auto& space = font.glyphs.at(' ');
    REQUIRE(space.advance > 0.0f);
    REQUIRE(space.size.x == 0.0f);
    REQUIRE(space.size.y == 0.0f);
}

TEST_CASE("BakedFont rejects a non-font blob") {
    std::vector<std::byte> garbage(128, std::byte { 0 });
    REQUIRE_THROWS(k2::BakedFont(garbage, 48.0f));
}

TEST_CASE("Font::measure computes line widths and block size") {
    auto ttf = read_file(K2_TEST_FONT);
    auto baked = k2::BakedFont { ttf, 48.0f };
    auto font = k2::Font { .glyphs = baked.glyphs,
        .ascent = baked.ascent,
        .descent = baked.descent,
        .line_gap = baked.line_gap,
        .bake_px = baked.bake_px };

    auto single = font.measure("AB", 48.0f);
    REQUIRE(single.line_widths.size() == 1);
    REQUIRE(single.width == font.glyphs.at('A').advance + font.glyphs.at('B').advance);
    REQUIRE(single.height == font.ascent - font.descent);

    auto multi = font.measure("AB\nA", 48.0f);
    REQUIRE(multi.line_widths.size() == 2);
    REQUIRE(multi.line_widths[0] == single.width);
    REQUIRE(multi.line_widths[1] == font.glyphs.at('A').advance);
    REQUIRE(multi.width == single.width);
    REQUIRE(multi.height == single.height + (font.ascent - font.descent + font.line_gap));

    // Rendering at half the bake size halves everything.
    auto half = font.measure("AB", 24.0f);
    REQUIRE(half.width == Catch::Approx(single.width / 2.0f));
    REQUIRE(half.height == Catch::Approx(single.height / 2.0f));
}
