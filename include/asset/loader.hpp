#pragma once

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
};

template <> Image AssetLoader::get<Image>(const Asset& asset);
template <> Texture2D AssetLoader::get<Texture2D>(const Asset& asset);
template <> Shader AssetLoader::get<Shader>(const Asset& asset);
template <> AssetBundle AssetLoader::get<AssetBundle>(const Asset& asset);
template <> Model AssetLoader::get<Model>(const Asset& asset, ResourceManager& resources);

}
