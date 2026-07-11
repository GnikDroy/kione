#include "asset/scheme.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace k2 {

std::unique_ptr<std::istream> AssetSchemeImpl<Asset::Scheme::file>::get_stream(const Asset& asset) {
    assert(Asset::Scheme::file == asset.get_scheme() && "Asset of different scheme.");
    return std::make_unique<std::ifstream>(asset.get_url_divisions().path.data(), std::ios::binary);
}

std::vector<std::uint8_t> AssetSchemeImpl<Asset::Scheme::file>::get_raw(const Asset& asset) {
    assert(Asset::Scheme::file == asset.get_scheme() && "Asset of different scheme.");
    std::ifstream file_stream { asset.get_url_divisions().path.data(), std::ios::binary };
    if (file_stream.fail()) {
        throw std::runtime_error(std::format("Cannot open file {}.", asset.get_url_divisions().path));
    }
    std::vector<std::uint8_t> raw;
    std::copy(
        (std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>(), std::back_inserter(raw));
    return raw;
}

std::unique_ptr<std::istream> AssetScheme::get_stream(const Asset& asset) {
    switch (asset.get_scheme()) {
    case Asset::Scheme::file: return AssetSchemeImpl<Asset::Scheme::file>::get_stream(asset);
    default: throw std::invalid_argument("Scheme not implemented");
    }
}

std::vector<std::uint8_t> AssetScheme::get_raw(const Asset& asset) {
    switch (asset.get_scheme()) {
    case Asset::Scheme::file: return AssetSchemeImpl<Asset::Scheme::file>::get_raw(asset);
    default: throw std::invalid_argument("Scheme not implemented");
    }
}

}
