#pragma once

#include "asset/asset.hpp"
#include <memory>

namespace k2 {
// Implements the scheme for all assets.
// Responsible for setting up communication with authority to gather the asset.
// Probably returns a file/network stream object, or the asset in memory.
template <Asset::Scheme T> struct AssetSchemeImpl;

template <> struct AssetSchemeImpl<Asset::Scheme::file> {
    static std::unique_ptr<std::istream> get_stream(const Asset& asset) {
        assert(Asset::Scheme::file == asset.get_scheme() && "Asset of different scheme.");
        return std::make_unique<std::ifstream>(asset.get_parts().path, std::ios::binary);
    }

    static std::vector<std::uint8_t> get_raw(const Asset& asset) {
        assert(Asset::Scheme::file == asset.get_scheme() && "Asset of different scheme.");
        std::ifstream file_stream { asset.get_parts().path, std::ios::binary };
        std::vector<std::uint8_t> raw;
        std::copy(std::istream_iterator<std::uint8_t>(file_stream), std::istream_iterator<std::uint8_t>(),
            std::back_inserter(raw));
        return raw;
    }
};

struct AssetScheme {
public:
    static std::unique_ptr<std::istream> get_stream(const Asset& asset) {
        switch (asset.get_scheme()) {
        case Asset::Scheme::file: return AssetSchemeImpl<Asset::Scheme::file>::get_stream(asset);
        default: throw std::invalid_argument("Scheme not implemented");
        }
    }

    static std::vector<std::uint8_t> get_raw(const Asset& asset) {
        switch (asset.get_scheme()) {
        case Asset::Scheme::file: return AssetSchemeImpl<Asset::Scheme::file>::get_raw(asset);
        default: std::invalid_argument("Scheme not implemented");
        }
    }
};

}