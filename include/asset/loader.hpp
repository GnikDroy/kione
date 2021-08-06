#pragma once

#include <stack>
#include <unordered_set>
#include <yaml-cpp/yaml.h>

#include "asset/scheme.hpp"
#include "rendering/model.hpp"
#include "serializers/asset/asset.hpp"

namespace k2 {

// A generic class responsible for loading / processing assets from memory into their respective resource.
// For example from memory to the GPU
// Capable of returning a resource, or a stream from an asset.
// Some assets might not need complete loading. Sound files for instance can be streamed directly from the filesystem.
// And it might be useful to just have the url in store.
template <Asset::Type T> class AssetLoader;

template <> struct AssetLoader<Asset::Type::Image> {
    static k2::Image get_resource(const Asset& asset) {
        auto traits = asset.get_traits();
        auto desired_channels = 0;
        if (traits.count("desired_channels")) {
            auto& desired_channels_sv = traits["desired_channels"];
            desired_channels
                = to_integer<int>(desired_channels_sv.data(), desired_channels_sv.data() + desired_channels_sv.size());
        }
        auto raw = AssetScheme::get_raw(asset);
        return k2::Image { raw, desired_channels };
    }
};

template <> struct AssetLoader<Asset::Type::Shader> {
    static k2::Shader get_resource(const Asset& asset) {
        auto traits = asset.get_traits();
        auto& type_sv = traits["type"];
        auto type = to_integer<std::uint32_t>(type_sv.data(), type_sv.data() + type_sv.size());
        auto stream = AssetScheme::get_stream(asset);

        std::string source { std::istreambuf_iterator<char>(*stream.get()), std::istreambuf_iterator<char>() };
        return k2::Shader { type, source };
    }
};

template <> struct AssetLoader<Asset::Type::Model> {
    // TODO: Custom Assimp IO logic.
    static k2::Model get_resource(const Asset& asset) { return k2::Model { asset.get_parts().path }; }
};

template <> struct AssetLoader<Asset::Type::AssetBundle> {
    static AssetBundle get_resource(const Asset& asset) {
        auto stream = AssetScheme::get_stream(asset);
        auto bundle = YAML::Load(*stream).as<AssetBundle>();
        bundle.assets[""] = asset;
        return bundle;
    }
};
}