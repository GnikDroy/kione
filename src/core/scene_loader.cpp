#include "core/scene_loader.hpp"

#include <format>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

#include "asset/loader.hpp"
#include "components/relation.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "core/logger.hpp"
#include "serializers/core/scene.hpp" // IWYU pragma: keep

namespace k2 {

Scene SceneLoader::load(const std::filesystem::path& path, ResourceManager& resources, const AssetRegistry& assets) {
    return load(YAML::LoadFile(path.string()), resources, assets);
}

static void load_referenced_textures(
    entt::registry& registry, ResourceManager& resources, const AssetRegistry& assets) {
    registry.view<SpriteComponent>().each([&](auto, const SpriteComponent& sprite) {
        const auto& handle = sprite.texture;
        if (handle.name.empty() || resources.contains<Texture2D>(handle.id)) {
            return;
        }

        auto it = assets.find(handle.id);
        if (it == assets.end() || it->second.second.type != Asset::Type::Image) {
            Log::core().warn(std::format("Scene references unknown texture asset '{}'", handle.name));
            return;
        }
        resources.set(handle.name, Texture2D { AssetLoader::get<Image>(it->second.second) });
    });
}

Scene SceneLoader::load(const YAML::Node& node, ResourceManager& resources, const AssetRegistry& assets) {
    if (!node.IsSequence()) {
        throw std::runtime_error("A scene must be a sequence of entities.");
    }

    Scene scene;
    auto& registry = scene.registry;
    registry.ctx().emplace<ResourceManager&>(resources);

    // Serialized entity ids are not stable across registries; entities are
    // recreated and every stored entity reference is remapped.
    std::unordered_map<entt::entity, entt::entity> remap;
    for (auto entity_node : node) {
        if (!entity_node.IsMap()) {
            throw std::runtime_error("A scene entity must be a map.");
        }
        remap[entity_node["Entity"].as<entt::entity>()] = registry.create();
    }

    auto remapped = [&](entt::entity old_entity) -> entt::entity {
        if (old_entity == entt::null) {
            return entt::null;
        }
        auto it = remap.find(old_entity);
        if (it == remap.end()) {
            Log::core().warn(std::format("Scene references unknown entity {}", entt::to_integral(old_entity)));
            return entt::null;
        }
        return it->second;
    };

    auto deserialize = [&]<class Component>(const auto& entity_node, const auto& label,
                           entt::entity entity) -> Component* {
        if (!entity_node[label].IsDefined()) {
            return nullptr;
        }
        if constexpr (std::is_empty_v<Component>) {
            registry.emplace<Component>(entity);
            return nullptr;
        } else {
            return &registry.emplace<Component>(entity, entity_node[label].template as<Component>());
        }
    };

    for (auto entity_node : node) {
        auto entity = remap.at(entity_node["Entity"].as<entt::entity>());

        deserialize.template operator()<TagComponent>(entity_node, "TagComponent", entity);
        deserialize.template operator()<TransformComponent>(entity_node, "TransformComponent", entity);
        if (auto* relation
            = deserialize.template operator()<RelationComponent>(entity_node, "RelationComponent", entity)) {
            relation->parent = remapped(relation->parent);
            relation->first = remapped(relation->first);
            relation->next = remapped(relation->next);
            relation->prev = remapped(relation->prev);
        }
        deserialize.template operator()<Camera>(entity_node, "Camera", entity);
        deserialize.template operator()<MainCamera>(entity_node, "MainCamera", entity);
        deserialize.template operator()<SpriteComponent>(entity_node, "SpriteComponent", entity);
        deserialize.template operator()<ScriptComponent>(entity_node, "ScriptComponent", entity);
    }

    load_referenced_textures(registry, resources, assets);
    return scene;
}

}
