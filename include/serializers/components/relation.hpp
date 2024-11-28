#pragma once
#include "components/relation.hpp"
#include "serializers/utils.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::RelationComponent> {
    static Node encode(const k2::RelationComponent& relation) {
        YAML::Node node;
        node["Parent"] = relation.parent;
        node["First"] = relation.first;
        node["Next"] = relation.next;
        node["Previous"] = relation.prev;
        node["Children"] = relation.children;
        return node;
    }

    static bool decode(const Node& node, k2::RelationComponent& relation) {
        if (!node.IsMap()) {
            return false;
        }
        relation.parent = node["Parent"].as<entt::entity>();
        relation.first = node["First"].as<entt::entity>();
        relation.next = node["Next"].as<entt::entity>();
        relation.prev = node["Previous"].as<entt::entity>();
        relation.children = node["Children"].as<std::size_t>();
        return true;
    }
};

}