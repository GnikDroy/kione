#pragma once

#include <expected>
#include <string>

#include "asset/asset.hpp"
#include "core/resources.hpp"

namespace k2 {

class Model;

// A generic class responsible for loading / processing assets from memory into their respective resource.
// For example from memory to the GPU
struct AssetLoader {
    template <class T> static T get(const Asset& asset);

    // Overload for assets that register sub-resources (e.g. model material
    // textures) into a resource manager while loading.
    template <class T> static T get(const Asset& asset, ResourceManager& resources);

    // Non-throwing variant: load failures come back as error messages.
    template <class T> [[nodiscard]] static std::expected<T, std::string> try_get(const Asset& asset) noexcept;
};

template <> Image AssetLoader::get<Image>(const Asset& asset);
template <> Texture2D AssetLoader::get<Texture2D>(const Asset& asset);
template <> Shader AssetLoader::get<Shader>(const Asset& asset);
template <> AssetBundle AssetLoader::get<AssetBundle>(const Asset& asset);
template <> SpriteAnimation AssetLoader::get<SpriteAnimation>(const Asset& asset);
template <> std::expected<Image, std::string> AssetLoader::try_get<Image>(const Asset& asset) noexcept;
template <> std::expected<SpriteAnimation, std::string> AssetLoader::try_get<SpriteAnimation>(
    const Asset& asset) noexcept;
template <> Model AssetLoader::get<Model>(const Asset& asset, ResourceManager& resources);

}
