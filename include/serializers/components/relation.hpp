#pragma once
#include "components/relation.hpp"
#include <cereal/cereal.hpp>

namespace cereal {
template <class Archive> void serialize(Archive& ar, k2::RelationComponent& relation) {
    ar(make_nvp("Parent", relation.parent));
    ar(make_nvp("First", relation.first));
    ar(make_nvp("Next", relation.next));
    ar(make_nvp("Prev", relation.prev));
    ar(make_nvp("Children", relation.children));
}
}