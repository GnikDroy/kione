#pragma once
#include "components/script.hpp"
#include "serializers/asset/asset_handle.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::ScriptComponent> {
    static Node encode(const k2::ScriptComponent& script) {
        YAML::Node node;
        node["Script"] = script.script;
        return node;
    }

    static bool decode(const Node& node, k2::ScriptComponent& script) {
        if (!node.IsMap()) {
            return false;
        }
        script.script = node["Script"].as<k2::AssetHandle>();
        return true;
    }
};
}
