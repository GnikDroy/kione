#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace k2 {
struct Scene;

struct AudioClip {
    std::uint32_t channels {};
    std::uint32_t sample_rate {};
    std::vector<float> frames {};

    AudioClip() = default;
    explicit AudioClip(std::span<const std::byte> encoded);
};

class AudioSystem {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    AudioSystem();
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    void play(const AudioClip& clip, float volume = 1.0f, float pitch = 1.0f);

    void update(Scene& scene);

    void stop_all();
};
}
