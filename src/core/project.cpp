#include "core/project.hpp"

#include "serializers/core/project.hpp" // IWYU pragma: keep

#include <array>
#include <format>
#include <fstream>
#include <string_view>
#include <utility>

namespace k2 {

// Every place an asset is referenced by name in a scene file: {component key, field key}.
static constexpr std::array<std::pair<std::string_view, std::string_view>, 6> scene_reference_fields { {
    { "SpriteComponent", "Texture" },
    { "SpriteLight", "Texture" },
    { "TextComponent", "Font" },
    { "AudioSourceComponent", "Clip" },
    { "ScriptComponent", "Script" },
    { "AnimationComponent", "Clip" },
} };

static AssetRegistry load_registry(const std::filesystem::path& project_file) {
    auto bundle_path = std::filesystem::relative(project_file);
    return AssetRegistryLoader::load({
        .url = std::format("file:///{}", bundle_path.generic_string()),
        .type = Asset::Type::AssetBundle,
    });
}

std::expected<Project, std::string> Project::load(const std::filesystem::path& project_file) noexcept {
    try {
        auto node = YAML::LoadFile(project_file.string());
        if (!node.IsMap()) {
            return std::unexpected("A project must be a map.");
        }

        Project project;
        project.file = std::filesystem::absolute(project_file);
        project.root = project.file.parent_path();
        project.name = node["name"].as<std::string>("");
        project.main_scene = node["main_scene"].as<std::string>();
        project.assets_node = node["assets"];
        project.assets = load_registry(project.file);
        return project;
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

std::expected<void, std::string> Project::reload_assets() noexcept {
    try {
        auto node = YAML::LoadFile(file.string());
        assets_node = node["assets"];
        assets = load_registry(file);
        return {};
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

std::expected<void, std::string> Project::save() const {
    if (file.empty()) {
        return std::unexpected("Project has no backing file to save to.");
    }

    std::ofstream out { file };
    out << YAML::Node { *this } << "\n";
    if (!out) {
        return std::unexpected(std::format("Failed to write project file: {}", file.string()));
    }
    return {};
}

namespace {
    std::filesystem::path asset_path(const Asset& asset) {
        return std::filesystem::path { std::string { asset.get_url_divisions().path } };
    }

    template <class Visitor> bool rewrite_yaml_file(const std::filesystem::path& path, Visitor&& visitor) {
        YAML::Node node = YAML::LoadFile(path.string());
        if (!std::forward<Visitor>(visitor)(node)) {
            return false;
        }
        std::ofstream out { path };
        out << node << "\n";
        return true;
    }

    bool rename_in_scene(YAML::Node& scene, const std::string& old_name, const std::string& new_name) {
        bool changed = false;
        for (auto entity : scene) {
            for (const auto& [component, field] : scene_reference_fields) {
                auto reference = entity[std::string { component }][std::string { field }];
                if (reference && reference.IsScalar() && reference.Scalar() == old_name) {
                    entity[std::string { component }][std::string { field }] = new_name;
                    changed = true;
                }
            }
        }
        return changed;
    }

}

std::expected<void, std::string> Project::add_asset(
    Asset::Type type, const std::string& name, const std::filesystem::path& file) {
    if (assets.contains(fnv1a(name))) {
        return std::unexpected(std::format("An asset named '{}' already exists.", name));
    }
    auto relative = std::filesystem::relative(std::filesystem::absolute(file), root);
    if (relative.empty() || relative.generic_string().starts_with("..")) {
        return std::unexpected("An asset must live inside the project root.");
    }

    if (!assets_node.IsDefined() || assets_node.IsNull()) {
        assets_node = YAML::Node { YAML::NodeType::Map };
    }
    assets_node[std::string { Asset { .url = {}, .type = type }.get_type_strv() }][name]
        = std::format("file:///{}", relative.generic_string());

    if (auto saved = save(); !saved) {
        return saved;
    }
    return reload_assets();
}

std::expected<void, std::string> Project::rename_asset(const std::string& old_name, const std::string& new_name) {
    if (new_name == old_name) {
        return {};
    }
    if (new_name.empty()) {
        return std::unexpected("An asset name cannot be empty.");
    }
    if (assets.contains(fnv1a(new_name))) {
        return std::unexpected(std::format("An asset named '{}' already exists.", new_name));
    }
    auto it = assets.find(fnv1a(old_name));
    if (it == assets.end()) {
        return std::unexpected(std::format("No asset named '{}'.", old_name));
    }
    const Asset& asset = it->second.second;

    try {
        for (const auto& [id, entry] : assets) {
            const auto& [entry_name, entry_asset] = entry;
            if (entry_asset.type == Asset::Type::Scene) {
                rewrite_yaml_file(asset_path(entry_asset),
                    [&](YAML::Node& scene) { return rename_in_scene(scene, old_name, new_name); });
            } else if (entry_asset.type == Asset::Type::Animation) {
                rewrite_yaml_file(asset_path(entry_asset), [&](YAML::Node& clip) {
                    if (clip["texture"] && clip["texture"].IsScalar() && clip["texture"].Scalar() == old_name) {
                        clip["texture"] = new_name;
                        return true;
                    }
                    return false;
                });
            }
        }
    } catch (const std::exception& e) {
        return std::unexpected(std::format("Failed to update references to '{}': {}", old_name, e.what()));
    }

    auto section = std::string { asset.get_type_strv() };
    auto url = assets_node[section][old_name];
    assets_node[section][new_name] = url;
    assets_node[section].remove(old_name);

    if (main_scene == old_name) {
        main_scene = new_name;
    }

    if (auto saved = save(); !saved) {
        return saved;
    }
    return reload_assets();
}

std::expected<void, std::string> Project::remove_asset(const std::string& name) {
    auto it = assets.find(fnv1a(name));
    if (it == assets.end()) {
        return std::unexpected(std::format("No asset named '{}'.", name));
    }
    if (name == main_scene) {
        return std::unexpected(std::format("Cannot remove '{}': it is the project's main scene.", name));
    }
    assets_node[std::string { it->second.second.get_type_strv() }].remove(name);

    if (auto saved = save(); !saved) {
        return saved;
    }
    return reload_assets();
}

}
