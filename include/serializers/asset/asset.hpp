#pragma once
#include <yaml-cpp/yaml.h>

#include "asset/asset.hpp"

namespace YAML {
template <> struct convert<k2::AssetBundle> {
    static Node encode(const k2::AssetBundle& registry) {
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> asset_type_map;
        for (auto& [asset_id, asset] : registry.assets) {
            asset_type_map[std::string(asset.get_type_strv())][asset_id] = asset.url;
        }

        YAML::Node type_map;
        for (auto& [type, map] : asset_type_map) {
            YAML::Node nodes;
            for (auto& [name, url] : map) {
                nodes[name] = url;
            }
            type_map[type] = nodes;
        }

        YAML::Node main;
        main["assets"] = type_map;
        return main;
    }

    static bool decode(const Node& node, k2::AssetBundle& registry) {
        // TODO: checking for VALID AssetBundle Format
        for (auto it = node["assets"].begin(); it != node["assets"].end(); it++) {
            auto type = k2::Asset::get_type(it->first.as<std::string>());
            auto& asset_map_it = it->second;

            for (auto asset_it = asset_map_it.begin(); asset_it != asset_map_it.end(); asset_it++) {
                auto&& name = asset_it->first.as<std::string>();
                auto&& url = asset_it->second.as<std::string>();
                registry.assets[name].type = type;
                registry.assets[name].url = std::move(url);
            }
        }
        return true;
    }
};
}