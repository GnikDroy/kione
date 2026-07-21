#pragma once
#include "components/tilemap.hpp"
#include "serializers/asset/asset_handle.hpp" // IWYU pragma: keep
#include "serializers/utils.hpp" // IWYU pragma: keep
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::TileMapComponent> {
    static Node encode(const k2::TileMapComponent& tilemap) {
        Node node;
        node["Tileset"] = tilemap.tileset;
        node["Size"] = tilemap.size;
        node["TileSize"] = tilemap.tile_size;
        // Empty cells are written as -1 to keep the file readable.
        Node tiles { NodeType::Sequence };
        tiles.SetStyle(YAML::EmitterStyle::Flow);
        for (auto tile : tilemap.tiles) {
            tiles.push_back(tile == k2::TileMapComponent::empty_tile ? -1 : int(tile));
        }
        node["Tiles"] = tiles;
        node["Color"] = tilemap.color;
        node["Unlit"] = tilemap.unlit;
        return node;
    }

    static bool decode(const Node& node, k2::TileMapComponent& tilemap) {
        if (!node.IsMap()) {
            return false;
        }
        tilemap.tileset = node["Tileset"].as<k2::AssetHandle>();
        if (node["Size"].IsDefined()) {
            tilemap.size = glm::max(node["Size"].as<glm::ivec2>(), glm::ivec2 {});
        }
        if (node["TileSize"].IsDefined()) {
            tilemap.tile_size = node["TileSize"].as<glm::vec2>();
        }
        for (const auto& entry : node["Tiles"]) {
            auto tile = entry.as<int>();
            tilemap.tiles.push_back(tile < 0 || tile > int(k2::TileMapComponent::empty_tile)
                    ? k2::TileMapComponent::empty_tile
                    : std::uint16_t(tile));
        }
        tilemap.tiles.resize(
            std::size_t(tilemap.size.x) * std::size_t(tilemap.size.y), k2::TileMapComponent::empty_tile);
        if (node["Color"].IsDefined()) {
            tilemap.color = node["Color"].as<glm::vec4>();
        }
        tilemap.unlit = node["Unlit"].as<bool>(false);
        return true;
    }
};
}
