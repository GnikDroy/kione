#include "core/animation_system.hpp"

#include <algorithm>
#include <cmath>

#include "components/animation.hpp"
#include "components/sprite.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"
#include "rendering/sprite_animation.hpp"

namespace k2 {

void AnimationSystem::update(Scene& scene, float dt) {
    auto& registry = scene.registry;
    auto& resources = registry.ctx().get<ResourceManager&>();

    registry.view<AnimationComponent, SpriteComponent>().each(
        [&](auto, AnimationComponent& animation, SpriteComponent& sprite) {
            const auto* clip = resources.try_get<SpriteAnimation>(animation.clip.id);
            if (clip == nullptr || clip->frames.empty()) {
                return;
            }

            if (animation.playing) {
                animation.elapsed += dt * animation.speed;
            }

            auto length = clip->length();
            auto time = std::clamp(animation.elapsed, 0.0f, length);
            if (clip->loop && length > 0.0f) {
                time = std::fmod(animation.elapsed, length);
                if (time < 0.0f) {
                    time += length;
                }
            } else if (time >= length && length > 0.0f) {
                animation.playing = false;
                animation.finished = true;
            }

            const auto& frame = clip->frame_at(time);
            sprite.texture = clip->texture;
            sprite.uv_rect = frame.uv;
            sprite.color = frame.color;
        });
}

}
