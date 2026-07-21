#include "asset/asset.hpp"

#include <algorithm>
#include <cassert>
#include <regex>
#include <stdexcept>

#include "core/utils.hpp"

namespace k2 {

std::string_view Asset::get_type_strv() const {
    switch (type) {
    case k2::Asset::Type::AssetBundle: return "AssetBundle";
    case k2::Asset::Type::Image: return "Image";
    case k2::Asset::Type::Shader: return "Shader";
    case k2::Asset::Type::Font: return "Font";
    case k2::Asset::Type::Model: return "Model";
    case k2::Asset::Type::Audio: return "Audio";
    case k2::Asset::Type::Data: return "Data";
    case k2::Asset::Type::Script: return "Script";
    case k2::Asset::Type::Animation: return "Animation";
    case k2::Asset::Type::Scene: return "Scene";
    case k2::Asset::Type::TileSet: return "TileSet";
    default: throw std::invalid_argument("Enum value reached an invalid state.");
    }
}

Asset::Type Asset::get_type(std::string_view type) {
    using namespace k2::literals;
    switch (k2::fnv1a(type.data(), type.size())) {
    case "AssetBundle"_fnv1a: return k2::Asset::Type::AssetBundle;
    case "Image"_fnv1a: return k2::Asset::Type::Image;
    case "Shader"_fnv1a: return k2::Asset::Type::Shader;
    case "Font"_fnv1a: return k2::Asset::Type::Font;
    case "Model"_fnv1a: return k2::Asset::Type::Model;
    case "Audio"_fnv1a: return k2::Asset::Type::Audio;
    case "Data"_fnv1a: return k2::Asset::Type::Data;
    case "Script"_fnv1a: return k2::Asset::Type::Script;
    case "Animation"_fnv1a: return k2::Asset::Type::Animation;
    case "Scene"_fnv1a: return k2::Asset::Type::Scene;
    case "TileSet"_fnv1a: return k2::Asset::Type::TileSet;
    default: throw std::invalid_argument("Invalid enum string received");
    }
}

Asset::URL Asset::get_url_divisions() const {
    URL parts;
    static const std::regex url_regex {
        R"(([\d\w]+?):)" // Scheme
        R"((?:\/\/([^\/]*))?)" // Authority
        R"((?:\/([^\?#]*))?)" // Path
        R"((?:\?([^#]*))?)" // Query
        R"((?:\#(.*)?)?)" // Fragment
    };

    std::smatch base;
    if (std::regex_match(url, base, url_regex)) {
        assert(base.size() == 6 && "Regular expression bad sub match number.");
        parts.scheme = { base[1].first, base[1].second };
        parts.authority = { base[2].first, base[2].second };
        parts.path = { base[3].first, base[3].second };
        parts.query = { base[4].first, base[4].second };
        parts.fragment = { base[5].first, base[5].second };
    }
    return parts;
}

Asset::Scheme Asset::get_scheme() const {
    using namespace k2::literals;
    auto scheme_strv = get_url_divisions().scheme;

    switch (fnv1a(scheme_strv.data(), scheme_strv.size())) {
    case "file"_fnv1a: return Scheme::file;
    default: throw std::invalid_argument("Scheme not implemented");
    }
}

std::unordered_map<std::string_view, std::string_view> Asset::get_traits() const {
    std::unordered_map<std::string_view, std::string_view> map;
    auto query = get_url_divisions().query;
    if (query.empty())
        return {};
    auto pairs = string_view_split(query, '&');

    for (auto pair_sv : pairs) {
        auto end = std::min(pair_sv.find('='), pair_sv.size());
        auto key = pair_sv.substr(0, end);

        auto next_start = std::min(end + 1, pair_sv.size());
        auto value = pair_sv.substr(next_start, pair_sv.size() - next_start);
        map[key] = value;
    }
    return map;
}

}
