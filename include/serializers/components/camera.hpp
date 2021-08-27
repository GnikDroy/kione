#pragma once
#include "components/camera.hpp"
#include "serializers/utils.hpp"
#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>

namespace cereal {
template <class Archive> void serialize(Archive& ar, k2::Camera::OrthographicTraits& ortho_traits) {
    ar(make_nvp("Left", ortho_traits.left));
    ar(make_nvp("Right", ortho_traits.right));
    ar(make_nvp("Top", ortho_traits.top));
    ar(make_nvp("Bottom", ortho_traits.bottom));
    ar(make_nvp("FarClip", ortho_traits.far_clip));
    ar(make_nvp("NearClip", ortho_traits.near_clip));
}
template <class Archive> void serialize(Archive& ar, k2::Camera::PerspectiveTraits& persp_traits) {
    ar(make_nvp("FOV", persp_traits.fov));
    ar(make_nvp("AspectRatio", persp_traits.aspect_ratio));
    ar(make_nvp("FarClip", persp_traits.far_clip));
    ar(make_nvp("NearClip", persp_traits.near_clip));
}
template <class Archive> void serialize(Archive& ar, k2::Camera& camera) {
    ar(make_nvp("Position", camera.position));
    ar(make_nvp("Target", camera.target));
    ar(make_nvp("Up", camera.up));
    ar(make_nvp("ProjectionTraits", camera.projection_traits));
}
}
