#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

#include "asset/asset_handle.hpp"
#include "core/utils.hpp"

namespace k2 {

struct SpriteAnimation {
    struct Frame {
        Rectf region { .x = 0.0f, .y = 0.0f, .w = 64.0f, .h = 64.0f };
        float duration = 0.1f;
        glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    AssetHandle texture {};
    std::vector<Frame> frames;
    bool loop = true;

    [[nodiscard]] float length() const {
        float total = 0.0f;
        for (const auto& frame : frames) {
            total += std::max(frame.duration, 0.0f);
        }
        return total;
    }

    [[nodiscard]] const Frame& frame_at(float time) const {
        float accumulated = 0.0f;
        for (const auto& frame : frames) {
            accumulated += std::max(frame.duration, 0.0f);
            if (time < accumulated) {
                return frame;
            }
        }
        return frames.back();
    }
};
}
