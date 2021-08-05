#pragma once
#include "asset/loader.hpp"

namespace k2 {
struct AssetRegistry {
    constexpr static inline std::string_view version = "0.0.1";
    std::unordered_map<std::string, Asset> assets;

    AssetRegistry(const Asset& base_asset, bool recursively = true) { load(base_asset, recursively); }

    // TODO: refactor
    AssetRegistry& load(const Asset& base_asset, bool recursively = true) {
        std::stack<Asset> stack;
        std::unordered_set<Asset::ID> visited;

        std::unordered_map<Asset::ID, Asset::ID> parent_map;
        std::unordered_map<Asset::ID, std::string> name_map;
        std::unordered_map<Asset::ID, std::string> path_map;

        stack.push(base_asset);

        while (!stack.empty()) {
            auto asset = stack.top();
            stack.pop();

            if (asset.get_scheme() == Asset::Scheme::file) {
                path_map[asset.get_id()] = asset.get_parts().path;
            }

            if (!visited.count(fnv1a(asset.url))) {
                visited.insert(fnv1a(asset.url));
                auto&& scope = get_scope(asset.get_id(), name_map, parent_map);
                auto&& base = get_path(asset.get_id(), path_map, parent_map);
                if (asset.get_scheme() == Asset::Scheme::file
                    && std::filesystem::path(asset.get_parts().path).is_relative()) {
                    patch_url(asset, "", base);
                    path_map[asset.get_id()] = asset.get_parts().path;
                }
                auto bundle = AssetLoader<Asset::Type::AssetBundle>::get_resource(asset);
                merge(bundle, scope, base);
                if (recursively) {
                    for (auto& [name, sub_asset] : bundle.assets) {
                        if (sub_asset.type == Asset::Type::AssetBundle && !visited.count(fnv1a(sub_asset.url))
                            && !name.empty()) {
                            stack.push(sub_asset);
                            auto&& id = sub_asset.get_id();
                            parent_map[id] = asset.get_id();
                            name_map[id] = name;
                        }
                    }
                }
            }
        }
        return *this;
    }

    // TODO: refactor
    AssetRegistry& merge(const AssetBundle& bundle, const std::string& scope, const std::filesystem::path& base) {
        namespace fs = std::filesystem;
        for (auto& [name, asset] : bundle.assets) {
            auto scoped_name = scope.empty() || name.empty() ? (scope + name) : (scope + "." + name);

            assets[scoped_name] = asset;

            // File paths can be relative so we patch that while including to registry.
            if (asset.get_scheme() == Asset::Scheme::file && fs::path(asset.get_parts().path).is_relative()) {
                auto asset_copy = asset;
                patch_url(asset_copy, name, base);
                assets[scoped_name] = asset_copy;
            }
        }
        return *this;
    }

private:
    // TODO: rename
    static void patch_url(Asset& asset, const std::string& name, const std::filesystem::path& base) {
        namespace fs = std::filesystem;
        auto new_path = base;
        auto&& parts = asset.get_parts();
        if (!name.empty()) {
            new_path /= fs::path(parts.path);
        } else {
            new_path /= fs::path(parts.path).filename();
        }
        auto new_url = std::string(parts.scheme) + ":" + // clang-format: no-join
            "//" + std::string(parts.authority) + //
            (parts.path.empty() ? "" : "/") + new_path.lexically_normal().string() + //
            (parts.query.empty() ? "" : "?") + std::string(parts.query) + //
            (parts.fragment.empty() ? "" : "#") + std::string(parts.fragment); //
        asset.url = std::move(new_url);
    }

    // TODO: rename
    static std::string get_scope(Asset::ID id, const std::unordered_map<Asset::ID, std::string>& name_map,
        const std::unordered_map<Asset::ID, Asset::ID>& parent_map) {
        std::string scope = "";
        for (; parent_map.count(id); id = parent_map.at(id)) {
            scope = name_map.at(id) + (scope.empty() ? "" : ("." + scope));
        }
        return scope;
    }

    // TODO: rename
    static std::filesystem::path get_path(Asset::ID id, const std::unordered_map<Asset::ID, std::string>& path_map,
        const std::unordered_map<Asset::ID, Asset::ID>& parent_map) {
        std::filesystem::path path;
        for (; path_map.count(id); id = parent_map.at(id)) {
            auto parent_path = std::filesystem::path(path_map.at(id)).parent_path();
            path = parent_path / path;
            if (!parent_path.is_relative() || !parent_map.count(id)) {
                break;
            }
        }
        return path;
    }
};
}
