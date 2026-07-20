#pragma once
#include "core/utils.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<entt::entity> {
    static Node encode(const entt::entity& entity) { return YAML::Node { entt::to_integral(entity) }; }

    static bool decode(const Node& node, entt::entity& entity) {
        entity = static_cast<entt::entity>(node.as<std::uint32_t>());
        return true;
    }
};

template <> struct convert<glm::vec2> {
    static Node encode(const glm::vec2& vec) {
        YAML::Node node;
        node.SetStyle(YAML::EmitterStyle::Flow);
        node.push_back(vec.x);
        node.push_back(vec.y);
        return node;
    }

    static bool decode(const Node& node, glm::vec2& vec) {
        if (!node.IsSequence() || node.size() != 2) {
            return false;
        }
        vec.x = node[0].as<float>();
        vec.y = node[1].as<float>();
        return true;
    }
};

template <> struct convert<glm::vec3> {
    static Node encode(const glm::vec3& vec) {
        YAML::Node node;
        node.SetStyle(YAML::EmitterStyle::Flow);
        node.push_back(vec.x);
        node.push_back(vec.y);
        node.push_back(vec.z);
        return node;
    }

    static bool decode(const Node& node, glm::vec3& vec) {
        if (!node.IsSequence() || node.size() != 3) {
            return false;
        }
        vec.x = node[0].as<float>();
        vec.y = node[1].as<float>();
        vec.z = node[2].as<float>();
        return true;
    }
};

template <> struct convert<glm::vec4> {
    static Node encode(const glm::vec4& vec) {
        YAML::Node node;
        node.SetStyle(YAML::EmitterStyle::Flow);
        node.push_back(vec.x);
        node.push_back(vec.y);
        node.push_back(vec.z);
        node.push_back(vec.w);
        return node;
    }

    static bool decode(const Node& node, glm::vec4& vec) {
        if (!node.IsSequence() || node.size() != 4) {
            return false;
        }
        vec.x = node[0].as<float>();
        vec.y = node[1].as<float>();
        vec.z = node[2].as<float>();
        vec.w = node[3].as<float>();
        return true;
    }
};

template <> struct convert<glm::quat> {
    static Node encode(const glm::quat& quat) {
        YAML::Node node;
        node.SetStyle(YAML::EmitterStyle::Flow);
        node.push_back(quat.x);
        node.push_back(quat.y);
        node.push_back(quat.z);
        node.push_back(quat.w);
        return node;
    }

    static bool decode(const Node& node, glm::quat& quat) {
        if (!node.IsSequence() || node.size() != 4) {
            return false;
        }
        quat.x = node[0].as<float>();
        quat.y = node[1].as<float>();
        quat.z = node[2].as<float>();
        quat.w = node[3].as<float>();
        return true;
    }
};

template <> struct convert<glm::mat4> {
    static Node encode(const glm::mat4& mat) {
        YAML::Node node;
        node.SetStyle(YAML::EmitterStyle::Flow);
        auto* ptr = glm::value_ptr(mat);
        for (auto i = 0; i < 4 * 4; i++) {
            node.push_back(ptr[i]);
        }
        return node;
    }

    static bool decode(const Node& node, glm::mat4& mat) {
        if (!node.IsSequence() || node.size() != 4 * 4) {
            return false;
        }

        auto* ptr = glm::value_ptr(mat);
        for (auto i = 0; i < 4 * 4; i++) {
            ptr[i] = node[i].as<float>();
        }
        return true;
    }
};

template <k2::arithmetic T> struct convert<k2::Rect<T>> {
    static Node encode(const k2::Rect<T>& rect) {
        YAML::Node node;
        node.SetStyle(YAML::EmitterStyle::Flow);
        node.push_back(rect.x);
        node.push_back(rect.y);
        node.push_back(rect.w);
        node.push_back(rect.h);
        return node;
    }

    static bool decode(const Node& node, k2::Rect<T>& rect) {
        if (!node.IsSequence() || node.size() != 4) {
            return false;
        }
        rect.x = node[0].as<float>();
        rect.y = node[1].as<float>();
        rect.w = node[2].as<float>();
        rect.h = node[3].as<float>();
        return true;
    }
};

}
