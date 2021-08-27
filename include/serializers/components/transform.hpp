#pragma once
#include "components/transform.hpp"
#include "serializers/utils.hpp"
#include <cereal/cereal.hpp>

namespace cereal {
template <class Archive> void serialize(Archive& ar, k2::TransformComponent t) {
    ar(make_nvp("Translation", t.translation));
    ar(make_nvp("Orientation", t.orientation));
    ar(make_nvp("Scale", t.scale));
}
}
