#include "asset/loader.hpp"

#include <iterator>
#include <span>
#include <stdexcept>
#include <string>

#include <yaml-cpp/yaml.h>

#include "asset/scheme.hpp"
#include "core/utils.hpp"
#include "rendering/model.hpp"
#include "serializers/asset/asset.hpp" // IWYU pragma: keep
#include "serializers/asset/sprite_animation.hpp" // IWYU pragma: keep

namespace k2 {

template <> Image AssetLoader::get<Image>(const Asset& asset) {
    if (asset.type != Asset::Type::Image) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    auto traits = asset.get_traits();
    auto desired_channels = 0;
    if (traits.count("desired_channels")) {
        auto& desired_channels_sv = traits["desired_channels"];
        desired_channels
            = to_number<int>(desired_channels_sv.data(), desired_channels_sv.data() + desired_channels_sv.size());
    }
    auto raw = AssetScheme::get_raw(asset);
    return k2::Image { raw, desired_channels };
}

template <> Texture2D AssetLoader::get<Texture2D>(const Asset& asset) {
    return k2::Texture2D { AssetLoader::get<k2::Image>(asset) };
}

template <> Shader AssetLoader::get<Shader>(const Asset& asset) {
    if (asset.type != Asset::Type::Shader) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    auto traits = asset.get_traits();
    auto& type_sv = traits["type"];
    auto type = to_number<std::uint32_t>(type_sv.data(), type_sv.data() + type_sv.size());
    auto stream = AssetScheme::get_stream(asset);

    std::string source { std::istreambuf_iterator<char>(*stream.get()), std::istreambuf_iterator<char>() };
    return { type, source };
}

template <> AssetBundle AssetLoader::get<AssetBundle>(const Asset& asset) {
    if (asset.type != Asset::Type::AssetBundle) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    auto stream = AssetScheme::get_stream(asset);
    auto bundle = YAML::Load(*stream).as<AssetBundle>();
    bundle.assets[""] = asset;
    return bundle;
}

template <> SpriteAnimation AssetLoader::get<SpriteAnimation>(const Asset& asset) {
    if (asset.type != Asset::Type::Animation) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    auto stream = AssetScheme::get_stream(asset);
    return YAML::Load(*stream).as<SpriteAnimation>();
}

template <> BakedFont AssetLoader::get<BakedFont>(const Asset& asset) {
    if (asset.type != Asset::Type::Font) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    auto raw = AssetScheme::get_raw(asset);
    return BakedFont { std::as_bytes(std::span { raw }) };
}

template <> AudioClip AssetLoader::get<AudioClip>(const Asset& asset) {
    if (asset.type != Asset::Type::Audio) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    auto raw = AssetScheme::get_raw(asset);
    return AudioClip { std::as_bytes(std::span { raw }) };
}

template <> Script AssetLoader::get<Script>(const Asset& asset) {
    if (asset.type != Asset::Type::Script) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    auto raw = AssetScheme::get_raw(asset);
    return Script { .source = std::string { raw.begin(), raw.end() } };
}

template <> std::expected<Image, std::string> AssetLoader::try_get<Image>(const Asset& asset) noexcept {
    try {
        return get<Image>(asset);
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

template <>
std::expected<SpriteAnimation, std::string> AssetLoader::try_get<SpriteAnimation>(const Asset& asset) noexcept {
    try {
        return get<SpriteAnimation>(asset);
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

template <> std::expected<BakedFont, std::string> AssetLoader::try_get<BakedFont>(const Asset& asset) noexcept {
    try {
        return get<BakedFont>(asset);
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

template <> std::expected<AudioClip, std::string> AssetLoader::try_get<AudioClip>(const Asset& asset) noexcept {
    try {
        return get<AudioClip>(asset);
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

template <> std::expected<Script, std::string> AssetLoader::try_get<Script>(const Asset& asset) noexcept {
    try {
        return get<Script>(asset);
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

template <> Model AssetLoader::get<Model>(const Asset& asset, ResourceManager& resources) {
    if (asset.type != Asset::Type::Model) {
        throw std::invalid_argument("Invalid Asset Type!");
    }
    return k2::Model { asset.get_url_divisions().path, resources };
}

}
