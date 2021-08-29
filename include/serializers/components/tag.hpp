#pragma once
#include "components/tag.hpp"
#include <cereal/cereal.hpp>

namespace cereal {
template <class Archive> void save(Archive& ar, const k2::TagComponent& tag_component) {
    ar(make_nvp("ID", std::string(tag_component.str())));
}

template <class Archive> void load(Archive& ar, k2::TagComponent& tag_component) {
    std::string tag;
    ar(make_nvp("ID", tag));
    std::memcpy(tag_component.tag.data(), tag.c_str(), std::min(tag.size(), tag_component.tag.size()));
    tag_component.tag[std::min(tag_component.tag.size(), tag.size())] = 0;
}
}
