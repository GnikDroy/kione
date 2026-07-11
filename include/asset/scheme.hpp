#pragma once

#include <istream>
#include <memory>
#include <vector>

#include "asset/asset.hpp"

namespace k2 {
// Implements the scheme for all assets.
// Responsible for setting up communication with authority to gather the asset.
// Probably returns a file/network stream object, or the asset in memory.
template <Asset::Scheme T> struct AssetSchemeImpl;

template <> struct AssetSchemeImpl<Asset::Scheme::file> {
    static std::unique_ptr<std::istream> get_stream(const Asset& asset);
    static std::vector<std::uint8_t> get_raw(const Asset& asset);
};

struct AssetScheme {
    static std::unique_ptr<std::istream> get_stream(const Asset& asset);
    static std::vector<std::uint8_t> get_raw(const Asset& asset);
};

}
