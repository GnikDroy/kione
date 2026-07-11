#pragma once
#include "asset.hpp"

#include <filesystem>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "core/resource_container.hpp"

namespace k2 {
using AssetRegistry = std::unordered_map<ResourceID, std::pair<std::string, Asset>>;

class AssetRegistryLoader {
    Asset root;
    AssetRegistry& registry;
    bool recurse {};

    std::stack<Asset> stack {};
    std::unordered_set<Asset::ID> visited {};

    std::unordered_map<Asset::ID, Asset::ID> node_to_parent {};
    std::unordered_map<Asset::ID, std::string> asset_id_to_name {};
    std::unordered_map<Asset::ID, std::string> asset_id_to_path {};

public:
    static AssetRegistry load(Asset root, bool recurse = true);

private:
    AssetRegistryLoader(Asset root, bool recurse, AssetRegistry& registry);

    void load();

    void process_asset_bundle(const Asset& asset);

    AssetBundle merge(const Asset& base_asset);

    static std::string join_name(const std::string& head, const std::string& tail);

    static std::string compute_new_path(const Asset& asset, const std::string& name, const std::string& base);

    static std::string build_url(std::string_view scheme, std::string_view authority,
        const std::filesystem::path& path, std::string_view query, std::string_view fragment);

    std::string get_fully_qualified_name(const Asset& asset);
};

}
