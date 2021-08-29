#pragma once
#include "components/camera.hpp"
#include "serializers/utils.hpp"
#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>

namespace cereal {
template <class Archive> void serialize(Archive& ar, k2::Camera::OrthographicTraits& orthographic_traits) {
    ar(make_nvp("Left", orthographic_traits.left));
    ar(make_nvp("Right", orthographic_traits.right));
    ar(make_nvp("Top", orthographic_traits.top));
    ar(make_nvp("Bottom", orthographic_traits.bottom));
    ar(make_nvp("FarClip", orthographic_traits.far_clip));
    ar(make_nvp("NearClip", orthographic_traits.near_clip));
}
template <class Archive> void serialize(Archive& ar, k2::Camera::PerspectiveTraits& perspective_traits) {
    ar(make_nvp("FOV", perspective_traits.fov));
    ar(make_nvp("AspectRatio", perspective_traits.aspect_ratio));
    ar(make_nvp("FarClip", perspective_traits.far_clip));
    ar(make_nvp("NearClip", perspective_traits.near_clip));
}
template <class Archive> void serialize(Archive& ar, k2::Camera& camera) {
    ar(make_nvp("Position", camera.position));
    ar(make_nvp("Target", camera.target));
    ar(make_nvp("Up", camera.up));
    ar(make_nvp("ProjectionTraits", camera.projection_traits));
}
}
