#pragma once
#include "core/utils.hpp"
#include <array>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cereal {
template <class Archive> void serialize(Archive& ar, glm::vec3& vec) {
    ar(cereal::make_size_tag(3));
    ar(vec.x, vec.y, vec.z);
}
template <class Archive> void serialize(Archive& ar, glm::vec4& vec) {
    ar(cereal::make_size_tag(4));
    ar(vec.x, vec.y, vec.z, vec.w);
}

template <class Archive> void serialize(Archive& ar, glm::quat& quaternion) {
    ar(cereal::make_size_tag(4));
    ar(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
}

template <class Archive> void serialize(Archive& ar, glm::mat4& mat) {
    auto* ptr = glm::value_ptr(mat);
    ar(cereal::make_size_tag(4 * 4));
    for (auto i = 0; i < 4 * 4; i++) {
        ar(ptr[i]);
    }
}

template <class Archive, k2::arithmetic T> void serialize(Archive& ar, k2::Rect<T>& rect) {
    ar(cereal::make_nvp("X", rect.x));
    ar(cereal::make_nvp("Y", rect.y));
    ar(cereal::make_nvp("W", rect.w));
    ar(cereal::make_nvp("H", rect.h));
}
}