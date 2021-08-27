#pragma once
#include "components/sprite.hpp"
#include "serializers/utils.hpp"
#include <cereal/cereal.hpp>

namespace cereal {
template <class Archive> void serialize(Archive& ar, k2::SpriteComponent& sprite) {
    ar(make_nvp("Texture", sprite.texture));
    ar(make_nvp("Color", sprite.color));
    ar(make_nvp("UV Rect", sprite.uv_rect));
}
}
