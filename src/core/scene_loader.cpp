#include "core/scene_loader.hpp"

#include <format>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

#include "asset/loader.hpp"
#include "asset/scheme.hpp"
#include "components/animation.hpp"
#include "components/light.hpp"
#include "components/relation.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "components/audio.hpp"
#include "components/text.hpp"
#include "core/logger.hpp"
#include "rendering/sprite_animation.hpp"
#include "serializers/core/scene.hpp" // IWYU pragma: keep

namespace k2 {

std::expected<Scene, std::string> SceneLoader::load(
    std::string_view name, ResourceManager& resources, const AssetRegistry& assets) noexcept {
    try {
        auto it = assets.find(ResourceManager::resolve(name));
        if (it == assets.end() || it->second.second.type != Asset::Type::Scene) {
            return std::unexpected(std::format("Unknown scene asset '{}'", name));
        }
        auto stream = AssetScheme::get_stream(it->second.second);
        return load(YAML::Load(*stream), resources, assets);
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

static void load_texture(const AssetHandle& handle, ResourceManager& resources, const AssetRegistry& assets) {
    if (handle.name.empty() || resources.contains<Texture2D>(handle.id)) {
        return;
    }

    auto it = assets.find(handle.id);
    if (it == assets.end() || it->second.second.type != Asset::Type::Image) {
        Log::core().warn(std::format("Scene references unknown texture asset '{}'", handle.name));
        return;
    }
    auto image = AssetLoader::try_get<Image>(it->second.second);
    if (!image) {
        Log::core().error(std::format("Failed to load texture '{}': {}", handle.name, image.error()));
        return;
    }
    resources.set(handle.name, Texture2D { *image });
}

static void load_textures(ResourceManager& resources, const AssetRegistry& assets) {
    for (const auto& [id, pair] : assets) {
        const auto& [name, asset] = pair;
        if (asset.type == Asset::Type::Image) {
            load_texture(AssetHandle { name }, resources, assets);
        }
    }
}

static void load_fonts(ResourceManager& resources, const AssetRegistry& assets) {
    for (const auto& [id, pair] : assets) {
        const auto& [name, asset] = pair;
        if (asset.type == Asset::Type::Font) {
            auto baked = AssetLoader::try_get<BakedFont>(asset);
            if (!baked) {
                Log::core().error(std::format("Failed to load font '{}': {}", name, baked.error()));
                continue;
            }
            resources.set(name,
                Texture2D { std::size_t(baked->width), std::size_t(baked->height),
                    std::span<const std::uint8_t> { baked->pixels }, GL_R8, false });
            resources.set(name,
                Font { .atlas = ResourceManager::resolve(name),
                    .glyphs = std::move(baked->glyphs),
                    .ascent = baked->ascent,
                    .descent = baked->descent,
                    .line_gap = baked->line_gap,
                    .bake_px = baked->bake_px });
        }
    }
}

static void load_audio_clips(ResourceManager& resources, const AssetRegistry& assets) {
    for (const auto& [id, pair] : assets) {
        const auto& [name, asset] = pair;
        if (asset.type == Asset::Type::Audio) {
            if (resources.contains<AudioClip>(id)) {
                continue;
            }
            auto clip = AssetLoader::try_get<AudioClip>(asset);
            if (!clip) {
                Log::core().error(std::format("Failed to load audio clip '{}': {}", name, clip.error()));
                continue;
            }
            resources.set(name, std::move(*clip));
        }
    }
}

static void load_animation_clips(entt::registry& registry, ResourceManager& resources, const AssetRegistry& assets) {
    for (const auto& [id, pair] : assets) {
        const auto& [name, asset] = pair;
        if (asset.type != Asset::Type::Animation) {
            continue;
        }
        auto clip = AssetLoader::try_get<SpriteAnimation>(asset);
        if (!clip) {
            Log::core().error(std::format("Failed to load animation clip '{}': {}", name, clip.error()));
            continue;
        }
        load_texture(clip->texture, resources, assets);
        resources.set(name, std::move(*clip));
    }

    registry.view<AnimationComponent>().each([&](auto, const AnimationComponent& animation) {
        if (!animation.clip.name.empty() && !resources.contains<SpriteAnimation>(animation.clip.id)) {
            Log::core().warn(std::format("Scene references unknown animation clip '{}'", animation.clip.name));
        }
    });
}

std::expected<Scene, std::string> SceneLoader::load(
    const YAML::Node& node, ResourceManager& resources, const AssetRegistry& assets) noexcept try {
    if (!node.IsSequence()) {
        return std::unexpected("A scene must be a sequence of entities.");
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

    auto deserialize
        = [&]<class Component>(const auto& entity_node, const auto& label, entt::entity entity) -> Component* {
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
        deserialize.template operator()<TextComponent>(entity_node, "TextComponent", entity);
        deserialize.template operator()<AudioSourceComponent>(entity_node, "AudioSourceComponent", entity);
        deserialize.template operator()<ColliderComponent>(entity_node, "ColliderComponent", entity);
        deserialize.template operator()<Environment>(entity_node, "Environment", entity);
        deserialize.template operator()<AnimationComponent>(entity_node, "AnimationComponent", entity);
        deserialize.template operator()<ScriptComponent>(entity_node, "ScriptComponent", entity);
        deserialize.template operator()<PointLight>(entity_node, "PointLight", entity);
        deserialize.template operator()<SpotLight>(entity_node, "SpotLight", entity);
        deserialize.template operator()<SpriteLight>(entity_node, "SpriteLight", entity);
    }

    load_textures(resources, assets);
    load_animation_clips(registry, resources, assets);
    load_fonts(resources, assets);
    load_audio_clips(resources, assets);
    return scene;
} catch (const std::exception& e) {
    return std::unexpected(e.what());
}

}
