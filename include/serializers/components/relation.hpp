#pragma once
#include "components/relation.hpp"
#include <cereal/cereal.hpp>

namespace cereal {
template <class Archive> void serialize(Archive& ar, k2::RelationComponent& relation) {
    ar(make_nvp("Parent", relation.parent));
}
}