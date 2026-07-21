#pragma once

#include <memory>

namespace k2 {
struct Scene;
struct AudioClip;

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
