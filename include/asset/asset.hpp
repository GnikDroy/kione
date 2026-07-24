#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "core/fnv.hpp"

namespace k2 {

struct Asset {
    using ID = decltype(fnv1a(""));
    enum class Scheme : uint8_t { file };
    enum class Type : uint8_t {
        AssetBundle,
        Image,
        Shader,
        Font,
        Model,
        Audio,
        Data,
        Script,
        Animation,
        Scene,
        TileSet
    };

    struct URL {
        std::string scheme, authority, path, query, fragment;
        bool operator==(const URL& url) const = default;
        bool operator!=(const URL& url) const = default;
    };

    std::string url;
    Type type;

    [[nodiscard]] ID get_id() const { return fnv1a(url); }

    [[nodiscard]] std::string_view get_type_strv() const;

    static Asset::Type get_type(std::string_view type);

    [[nodiscard]] URL get_url_divisions() const;

    [[nodiscard]] Scheme get_scheme() const;

    [[nodiscard]] std::unordered_map<std::string, std::string> get_traits() const;
};

struct AssetBundle {
    std::unordered_map<std::string, Asset> assets;
};

}
