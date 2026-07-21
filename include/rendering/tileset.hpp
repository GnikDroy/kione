#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <unordered_map>

#include <glm/glm.hpp>

#include "asset/asset_handle.hpp"
#include "core/utils.hpp"

namespace k2 {

struct Tile {
    Rectf uv {};
};

struct TileSet {
    AssetHandle texture {};
    glm::ivec2 tile_size { 32, 32 };
    glm::ivec2 margin {};
    glm::ivec2 spacing {};
    std::unordered_map<std::size_t, Tile> tiles {};

    glm::ivec2 texture_size {};

    [[nodiscard]] int columns() const {
        return std::max(1, (texture_size.x - margin.x + spacing.x) / (tile_size.x + spacing.x));
    }

    [[nodiscard]] int rows() const {
        return std::max(1, (texture_size.y - margin.y + spacing.y) / (tile_size.y + spacing.y));
    }

    [[nodiscard]] bool contains(int index) const { return index >= 0 && index < columns() * rows(); }

    [[nodiscard]] int size() const { return columns() * rows(); }

    // The uv rect is inset half a texel so linear filtering never samples a neighbour or the spacing.
    [[nodiscard]] Tile operator[](int index) const {
        if (!contains(index)) {
            throw std::out_of_range("TileSet: tile index out of bounds");
        }
        auto it = tiles.find(std::size_t(index));
        auto tile = it != tiles.end() ? it->second : Tile {};
        auto cols = columns();
        auto origin = margin + glm::ivec2 { index % cols, index / cols } * (tile_size + spacing);
        tile.uv = {
            .x = (float(origin.x) + 0.5f) / float(texture_size.x),
            .y = (float(texture_size.y - origin.y - tile_size.y) + 0.5f) / float(texture_size.y),
            .w = float(tile_size.x - 1) / float(texture_size.x),
            .h = float(tile_size.y - 1) / float(texture_size.y),
        };
        return tile;
    }

    struct Iterator {
        using iterator_category = std::input_iterator_tag;
        using value_type = Tile;
        using difference_type = std::ptrdiff_t;

        const TileSet* tileset {};
        int index {};

        Tile operator*() const { return (*tileset)[index]; }
        Iterator& operator++() {
            ++index;
            return *this;
        }
        Iterator operator++(int) {
            auto copy = *this;
            ++index;
            return copy;
        }
        bool operator==(const Iterator&) const = default;
    };

    [[nodiscard]] Iterator begin() const { return { .tileset = this, .index = 0 }; }
    [[nodiscard]] Iterator end() const { return { .tileset = this, .index = size() }; }
};
}
