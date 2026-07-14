#include "asset/asset_registry.hpp"

#include <format>
#include <ranges>
#include <stdexcept>

#include "asset/loader.hpp"
#include "core/logger.hpp"

namespace k2 {

AssetRegistry AssetRegistryLoader::load(Asset root, bool recurse) {
    AssetRegistry registry;
    AssetRegistryLoader loader { root, recurse, registry };
    return registry;
}

AssetRegistryLoader::AssetRegistryLoader(Asset root, bool recurse, AssetRegistry& registry)
    : root { root }
    , registry { registry }
    , recurse { recurse } {
    if (root.type != k2::Asset::Type::AssetBundle) {
        throw std::runtime_error(std::format("Root asset: {} must be an asset bundle", root.url));
    }

    load();

    k2::Log::core().info(std::format("Loaded asset: {}", root.url));
}

void AssetRegistryLoader::load() {
    stack.push(root);
    while (!stack.empty()) {
        auto asset = stack.top();
        stack.pop();

        if (!visited.contains(fnv1a(asset.url))) {
            visited.insert(fnv1a(asset.url));
            process_asset_bundle(asset);
        }
    }
}

void AssetRegistryLoader::process_asset_bundle(const Asset& asset) {
    namespace fs = std::filesystem;

    // Strip filename from asset bundle url when storing path
    auto parts = asset.get_url_divisions();
    auto path = fs::path(parts.path).parent_path().string();
    asset_id_to_path[asset.get_id()] = path;

    auto bundle = merge(asset);

    if (recurse) {
        auto recursable_assets = bundle.assets | std::views::filter([&](const auto& pair) {
            auto& [name, asset] = pair;
            bool is_asset_bundle = asset.type == Asset::Type::AssetBundle;
            bool is_not_visited = !visited.contains(fnv1a(asset.url));
            bool is_not_self = !name.empty();
            return is_asset_bundle && is_not_visited && is_not_self;
        });

        for (const auto& [name, child_asset] : recursable_assets) {
            stack.push(child_asset);
            auto id = child_asset.get_id();
            node_to_parent[id] = asset.get_id();
            asset_id_to_name[id] = name;
        }
    }
}

AssetBundle AssetRegistryLoader::merge(const Asset& base_asset) {
    namespace fs = std::filesystem;

    auto bundle = AssetLoader::get<AssetBundle>(base_asset);
    auto base_name = get_fully_qualified_name(base_asset);

    for (auto& [child_name, child_asset] : bundle.assets) {
        auto full_name = join_name(base_name, child_name);

        // File paths can be relative, therefore,
        // we make sure base path is taken into consideration
        if (fs::path(child_asset.get_url_divisions().path).is_relative()) {
            auto base_path = asset_id_to_path[base_asset.get_id()];
            child_asset.url = compute_new_path(child_asset, child_name, base_path);
        }

        registry[fnv1a(full_name)] = { full_name, child_asset };
    }
    return bundle;
}

std::string AssetRegistryLoader::join_name(const std::string& head, const std::string& tail) {
    if (head.empty() || tail.empty()) {
        return head + tail;
    }
    return head + "." + tail;
}

std::string AssetRegistryLoader::compute_new_path(const Asset& asset, const std::string& name, const std::string& base) {
    namespace fs = std::filesystem;
    auto parts = asset.get_url_divisions();

    fs::path new_path = base;

    if (!name.empty()) {
        new_path /= fs::path(parts.path);
    } else {
        new_path /= fs::path(parts.path).filename();
    }

    return build_url(parts.scheme, parts.authority, new_path, parts.query, parts.fragment);
}

std::string AssetRegistryLoader::build_url(std::string_view scheme, std::string_view authority,
    const std::filesystem::path& path, std::string_view query, std::string_view fragment) {
    auto new_path = path.empty() ? "" : "/" + path.lexically_normal().string();
    auto new_query = query.empty() ? "" : "?" + std::string(query);
    auto new_fragment = fragment.empty() ? "" : "#" + std::string(fragment);

    return std::format("{}://{}{}{}{}", scheme, authority, new_path, new_query, new_fragment);
}

std::string AssetRegistryLoader::get_fully_qualified_name(const Asset& asset) {
    auto id = asset.get_id();
    std::string full_name = asset_id_to_name[id];

    while (node_to_parent.contains(id)) {
        id = node_to_parent.at(id);
        full_name = join_name(asset_id_to_name[id], full_name);
    }
    return full_name;
}

}
