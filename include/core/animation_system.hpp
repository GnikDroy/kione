#pragma once

namespace k2 {
struct Scene;

struct AnimationSystem {
    static void update(Scene& scene, float dt);
};
}
