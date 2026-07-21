#include <catch2/catch_all.hpp>

#include <ranges>

#include "rendering/tileset.hpp"

#include "components/tilemap.hpp"
#include "serializers/asset/tileset.hpp"
#include "serializers/components/tilemap.hpp"

TEST_CASE("uv_for addresses a plain grid", "[tileset]") {
    k2::TileSet tileset;
    tileset.tile_size = { 16, 16 };
    tileset.texture_size = { 64, 32 }; // 4 columns, 2 rows

    REQUIRE(tileset.columns() == 4);

    // Tile row 0 is the top of the sheet; textures are v-flipped on load, and
    // rects are inset half a texel against filter bleed.
    auto top_left = tileset[0].uv;
    REQUIRE(top_left.x == Catch::Approx(0.5f / 64.0f));
    REQUIRE(top_left.y == Catch::Approx(16.5f / 32.0f));
    REQUIRE(top_left.w == Catch::Approx(15.0f / 64.0f));
    REQUIRE(top_left.h == Catch::Approx(15.0f / 32.0f));

    auto second_row = tileset[5].uv; // column 1, row 1
    REQUIRE(second_row.x == Catch::Approx(16.5f / 64.0f));
    REQUIRE(second_row.y == Catch::Approx(0.5f / 32.0f));
}

TEST_CASE("uv_for accounts for margin and spacing", "[tileset]") {
    k2::TileSet tileset;
    tileset.tile_size = { 16, 16 };
    tileset.margin = { 2, 2 };
    tileset.spacing = { 1, 1 };
    tileset.texture_size = { 70, 37 };

    REQUIRE(tileset.columns() == 4); // (70 - 2 + 1) / (16 + 1)

    auto first = tileset[0].uv; // pixel origin (2, 2)
    REQUIRE(first.x == Catch::Approx(2.5f / 70.0f));
    REQUIRE(first.y == Catch::Approx(19.5f / 37.0f)); // 37 - 2 - 16, plus the inset
    REQUIRE(first.w == Catch::Approx(15.0f / 70.0f));
    REQUIRE(first.h == Catch::Approx(15.0f / 37.0f));

    auto second_row = tileset[5].uv; // column 1, row 1: pixel origin (19, 19)
    REQUIRE(second_row.x == Catch::Approx(19.5f / 70.0f));
    REQUIRE(second_row.y == Catch::Approx(2.5f / 37.0f)); // 37 - 19 - 16, plus the inset
}

TEST_CASE("operator[] always yields a tile with computed uv", "[tileset]") {
    k2::TileSet tileset;
    tileset.tile_size = { 16, 16 };
    tileset.texture_size = { 64, 32 };
    tileset.tiles[1] = k2::Tile {}; // stored metadata entry: uv is still computed

    REQUIRE(tileset[1].uv.x == Catch::Approx(16.5f / 64.0f));
    REQUIRE(tileset[2].uv.x == Catch::Approx(32.5f / 64.0f)); // no stored entry

    REQUIRE(tileset.rows() == 2);
    REQUIRE(tileset.contains(7));
    REQUIRE_FALSE(tileset.contains(8));
    REQUIRE_THROWS_AS(tileset[8], std::out_of_range);
    REQUIRE_THROWS_AS(tileset[-1], std::out_of_range);
}

TEST_CASE("TileSet is an iterable range over the sheet", "[tileset]") {
    static_assert(std::ranges::input_range<k2::TileSet>);

    k2::TileSet tileset;
    tileset.tile_size = { 16, 16 };
    tileset.texture_size = { 64, 32 };

    REQUIRE(tileset.size() == 8);
    REQUIRE(std::ranges::distance(tileset) == 8);

    int index = 0;
    for (auto tile : tileset) {
        REQUIRE(tile.uv.x == Catch::Approx(tileset[index].uv.x));
        REQUIRE(tile.uv.y == Catch::Approx(tileset[index].uv.y));
        index++;
    }
    REQUIRE(index == 8);
}

TEST_CASE("TileSet serializer round-trips and omits derived data", "[tileset]") {
    k2::TileSet original;
    original.texture.set("dungeon");
    original.tile_size = { 16, 24 };
    original.margin = { 2, 3 };
    original.spacing = { 1, 0 };
    original.texture_size = { 128, 128 }; // loader-derived; must not serialize
    original.tiles[7] = k2::Tile {};

    auto node = YAML::Node { original };
    REQUIRE_FALSE(node["texture_size"].IsDefined());

    auto loaded = node.as<k2::TileSet>();
    REQUIRE(loaded.texture.name == "dungeon");
    REQUIRE(loaded.tile_size == glm::ivec2 { 16, 24 });
    REQUIRE(loaded.margin == glm::ivec2 { 2, 3 });
    REQUIRE(loaded.spacing == glm::ivec2 { 1, 0 });
    REQUIRE(loaded.texture_size == glm::ivec2 {});
    REQUIRE(loaded.tiles.size() == 1);
    REQUIRE(loaded.tiles.contains(7));
}

TEST_CASE("TileSet decode defaults missing fields", "[tileset]") {
    auto loaded = YAML::Load("texture: dungeon\ntile_size: [8, 8]").as<k2::TileSet>();
    REQUIRE(loaded.margin == glm::ivec2 {});
    REQUIRE(loaded.spacing == glm::ivec2 {});
    REQUIRE(loaded.tiles.empty());

    // Degenerate tile sizes are clamped rather than dividing by zero later.
    auto clamped = YAML::Load("texture: dungeon\ntile_size: [0, -3]").as<k2::TileSet>();
    REQUIRE(clamped.tile_size == glm::ivec2 { 1, 1 });
}

TEST_CASE("TileMapComponent serializer round-trips", "[tileset]") {
    constexpr auto empty = k2::TileMapComponent::empty_tile;
    k2::TileMapComponent original;
    original.tileset.set("dungeon_tiles");
    original.size = { 3, 2 };
    original.tile_size = { 24.0f, 16.0f };
    original.tiles = { 0, empty, 2, 3, empty, 5 };
    original.color = { 0.5f, 1.0f, 1.0f, 0.75f };
    original.unlit = true;

    auto node = YAML::Node { original };
    REQUIRE(node["Tiles"][1].as<int>() == -1); // empties stay readable in the file

    auto loaded = node.as<k2::TileMapComponent>();
    REQUIRE(loaded.tileset.name == "dungeon_tiles");
    REQUIRE(loaded.size == glm::ivec2 { 3, 2 });
    REQUIRE(loaded.tile_size == glm::vec2 { 24.0f, 16.0f });
    REQUIRE(loaded.tiles == std::vector<std::uint16_t> { 0, empty, 2, 3, empty, 5 });
    REQUIRE(loaded.color.a == 0.75f);
    REQUIRE(loaded.unlit);
}

TEST_CASE("TileMapComponent operator[] indexes the grid", "[tileset]") {
    k2::TileMapComponent tilemap;
    tilemap.size = { 3, 2 };
    tilemap.tiles = { 0, 1, 2, 3, 4, 5 };

    REQUIRE(tilemap[2, 0] == 2);
    REQUIRE(tilemap[0, 1] == 3);

    tilemap[1, 1] = 9;
    const auto& read_only = tilemap;
    REQUIRE(read_only[1, 1] == 9);
    REQUIRE(tilemap.tiles == std::vector<std::uint16_t> { 0, 1, 2, 3, 9, 5 });
}

TEST_CASE("TileMapComponent resize preserves the overlap and clears new cells", "[tileset]") {
    constexpr auto empty = k2::TileMapComponent::empty_tile;
    k2::TileMapComponent tilemap;
    tilemap.size = { 3, 2 };
    tilemap.tiles = { 0, 1, 2, 3, 4, 5 };

    // Grow: the old 3x2 block stays put (top-left), the rest is empty.
    tilemap.resize({ 4, 3 });
    REQUIRE(tilemap.size == glm::ivec2 { 4, 3 });
    REQUIRE(tilemap.tiles
        == std::vector<std::uint16_t> { 0, 1, 2, empty, 3, 4, 5, empty, empty, empty, empty, empty });

    // Shrink back: only the surviving top-left rectangle is kept.
    tilemap.resize({ 2, 2 });
    REQUIRE(tilemap.size == glm::ivec2 { 2, 2 });
    REQUIRE(tilemap.tiles == std::vector<std::uint16_t> { 0, 1, 3, 4 });

    // Degenerate sizes clamp to empty rather than allocating garbage.
    tilemap.resize({ -1, 5 });
    REQUIRE(tilemap.size == glm::ivec2 { 0, 5 });
    REQUIRE(tilemap.tiles.empty());
}

TEST_CASE("TileMapComponent operator[] is bounds-checked", "[tileset]") {
    k2::TileMapComponent tilemap;
    tilemap.size = { 2, 2 };
    tilemap.tiles = { 0, 1, 2, 3 };

    REQUIRE(tilemap.contains(1, 1));
    REQUIRE_FALSE(tilemap.contains(2, 0));
    REQUIRE_THROWS_AS((tilemap[-1, 0]), std::out_of_range);
    REQUIRE_THROWS_AS((tilemap[0, 2]), std::out_of_range);
    REQUIRE_THROWS_AS((std::as_const(tilemap)[2, 0]), std::out_of_range);
}
