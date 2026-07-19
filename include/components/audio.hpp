#pragma once

#include "asset/asset_handle.hpp"

namespace k2 {

struct AudioSourceComponent {
    AssetHandle clip {};
    float volume { 1.0f };
    float pitch { 1.0f };
    bool looping { false };
    bool play_on_create { false };
};

}
