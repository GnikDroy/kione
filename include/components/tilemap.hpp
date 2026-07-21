#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>

#include "asset/asset_handle.hpp"

namespace k2 {

struct TileMapComponent {
    static constexpr std::uint16_t empty_tile = 0xffff;

    AssetHandle tileset {};
    glm::ivec2 size {};
    glm::vec2 tile_size { 32.0f, 32.0f };
    std::vector<std::uint16_t> tiles {};
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
    bool unlit { false };

    [[nodiscard]] bool contains(int x, int y) const { return x >= 0 && y >= 0 && x < size.x && y < size.y; }

    void resize(glm::ivec2 new_size) {
        new_size = glm::max(new_size, glm::ivec2 { 0, 0 });
        std::vector<std::uint16_t> next(std::size_t(new_size.x) * std::size_t(new_size.y), empty_tile);
        for (int y = 0; y < std::min(size.y, new_size.y); y++) {
            for (int x = 0; x < std::min(size.x, new_size.x); x++) {
                next[std::size_t(y) * std::size_t(new_size.x) + std::size_t(x)] = (*this)[x, y];
            }
        }
        tiles = std::move(next);
        size = new_size;
    }

    [[nodiscard]] std::uint16_t& operator[](int x, int y) {
        if (!contains(x, y)) {
            throw std::out_of_range("TileMapComponent: tile out of bounds");
        }
        return tiles[std::size_t(y) * std::size_t(size.x) + std::size_t(x)];
    }

    [[nodiscard]] std::uint16_t operator[](int x, int y) const {
        if (!contains(x, y)) {
            throw std::out_of_range("TileMapComponent: tile out of bounds");
        }
        return tiles[std::size_t(y) * std::size_t(size.x) + std::size_t(x)];
    }
};
}
