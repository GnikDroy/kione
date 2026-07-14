#pragma once
#include "asset/asset_handle.hpp"

namespace k2 {

struct AnimationComponent {
    AssetHandle clip {};
    float speed = 1.0f;
    bool playing = true;
    bool finished = false;
    float elapsed = 0.0f;
};
}
