#pragma once
#include "asset/asset_handle.hpp"

#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::AssetHandle> {
    static Node encode(const k2::AssetHandle& handle) { return Node(handle.name); }

    static bool decode(const Node& node, k2::AssetHandle& handle) {
        if (!node.IsScalar()) {
            return false;
        }
        handle.set(node.Scalar());
        return true;
    }
};
}
