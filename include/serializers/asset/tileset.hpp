#pragma once
#include <yaml-cpp/yaml.h>

#include "rendering/tileset.hpp"
#include "serializers/asset/asset_handle.hpp" // IWYU pragma: keep
#include "serializers/utils.hpp" // IWYU pragma: keep

namespace YAML {
template <> struct convert<k2::Tile> {
    static Node encode(const k2::Tile&) { return Node(true); }

    static bool decode(const Node&, k2::Tile&) { return true; }
};

template <> struct convert<k2::TileSet> {
    static Node encode(const k2::TileSet& tileset) {
        Node node;
        node["texture"] = tileset.texture;
        node["tile_size"] = tileset.tile_size;
        node["margin"] = tileset.margin;
        node["spacing"] = tileset.spacing;
        if (!tileset.tiles.empty()) {
            Node tiles;
            for (const auto& [index, tile] : tileset.tiles) {
                tiles[index] = tile;
            }
            node["tiles"] = tiles;
        }
        return node;
    }

    static bool decode(const Node& node, k2::TileSet& tileset) {
        if (!node.IsMap()) {
            return false;
        }
        tileset.texture = node["texture"].as<k2::AssetHandle>();
        if (node["tile_size"].IsDefined()) {
            tileset.tile_size = node["tile_size"].as<glm::ivec2>();
        }
        tileset.tile_size = glm::max(tileset.tile_size, glm::ivec2 { 1, 1 });
        if (node["margin"].IsDefined()) {
            tileset.margin = node["margin"].as<glm::ivec2>();
        }
        if (node["spacing"].IsDefined()) {
            tileset.spacing = node["spacing"].as<glm::ivec2>();
        }
        for (const auto& entry : node["tiles"]) {
            tileset.tiles[entry.first.as<int>()] = entry.second.as<k2::Tile>();
        }
        return true;
    }
};
}
