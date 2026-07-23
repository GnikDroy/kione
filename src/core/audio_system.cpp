#include "core/audio_system.hpp"

#include <algorithm>
#include <format>
#include <list>
#include <ranges>
#include <vector>

#include "components/audio.hpp"
#include "core/audio_clip.hpp"
#include "core/logger.hpp"
#include "core/miniaudio.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"

namespace k2 {

namespace {
    constexpr std::size_t max_voices = 64;

    struct AudioStarted { };
    struct AudioSystemAttached { };
}

struct AudioSystem::Impl {
    ma_engine engine {};
    bool ready { false };

    struct Voice {
        ma_audio_buffer buffer {};
        ma_sound sound {};
        entt::entity owner { entt::null };
        std::shared_ptr<const std::vector<float>> pcm {};
    };

    // std::list: voices must not move — ma_sound holds a pointer to its buffer.
    std::list<Voice> voices;
    bool voice_cap_warned { false };

    Impl() {
        if (ma_engine_init(nullptr, &engine) != MA_SUCCESS) {
            Log::core().error("Audio device init failed; sound is disabled");
            return;
        }
        ready = true;
    }

    ~Impl() {
        stop_all();
        if (ready) {
            ma_engine_uninit(&engine);
        }
    }

    void play(const AudioClip& clip, float volume, float pitch, bool loop, entt::entity owner) {
        if (!ready || clip.channels == 0 || !clip.frames || clip.frames->empty()) {
            return;
        }
        if (voices.size() >= max_voices) {
            if (!voice_cap_warned) {
                voice_cap_warned = true;
                Log::core().warn(std::format("Voice cap of {} reached; dropping sounds", max_voices));
            }
            return;
        }
        auto& voice = voices.emplace_back();
        voice.owner = owner;
        voice.pcm = clip.frames;
        auto buffer_config = ma_audio_buffer_config_init(
            ma_format_f32, clip.channels, voice.pcm->size() / clip.channels, voice.pcm->data(), nullptr);
        buffer_config.sampleRate = clip.sample_rate;
        if (ma_audio_buffer_init(&buffer_config, &voice.buffer) != MA_SUCCESS) {
            voices.pop_back();
            return;
        }
        if (ma_sound_init_from_data_source(
                &engine, &voice.buffer, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, &voice.sound)
            != MA_SUCCESS) {
            ma_audio_buffer_uninit(&voice.buffer);
            voices.pop_back();
            return;
        }
        ma_sound_set_volume(&voice.sound, volume);
        ma_sound_set_pitch(&voice.sound, std::max(0.01f, pitch));
        ma_sound_set_looping(&voice.sound, loop);
        ma_sound_start(&voice.sound);
    }

    void release(Voice& voice) {
        ma_sound_uninit(&voice.sound);
        ma_audio_buffer_uninit(&voice.buffer);
    }

    template <class Predicate> void release_if(Predicate&& should_release) {
        std::erase_if(voices, [&](Voice& voice) {
            if (!should_release(voice)) {
                return false;
            }
            release(voice);
            return true;
        });
    }

    void stop_all() {
        release_if([](const Voice&) { return true; });
    }

    void attach_scene(entt::registry& registry) {
        if (registry.ctx().contains<AudioSystemAttached>()) {
            return;
        }
        registry.ctx().emplace<AudioSystemAttached>();
        registry.on_destroy<AudioSourceComponent>().connect<&Impl::on_source_destroyed>(*this);
        stop_owned();
    }

    void stop_owned() {
        release_if([](const Voice& voice) { return voice.owner != entt::null; });
    }

    void on_source_destroyed(entt::registry& registry, entt::entity entity) {
        registry.remove<AudioStarted>(entity);
        release_if([entity](const Voice& voice) { return voice.owner == entity; });
    }
};

AudioSystem::AudioSystem()
    : impl { std::make_unique<Impl>() } { }

AudioSystem::~AudioSystem() = default;

void AudioSystem::play(const AudioClip& clip, float volume, float pitch) {
    impl->play(clip, volume, pitch, false, entt::null);
}

void AudioSystem::stop_all() { impl->stop_all(); }

void AudioSystem::update(Scene& scene) {
    if (!impl->ready) {
        return;
    }
    auto& registry = scene.registry;
    impl->attach_scene(registry);

    // Reap finished one-shots; no entity means no destroy signal covers them.
    impl->release_if([&](Impl::Voice& voice) {
        bool owner_died = voice.owner != entt::null && !registry.valid(voice.owner);
        return owner_died || (!ma_sound_is_looping(&voice.sound) && ma_sound_at_end(&voice.sound));
    });

    // Follow component volume/pitch so scripts can change them live.
    auto owned = impl->voices | std::views::filter([](auto& voice) { return voice.owner != entt::null; });
    for (auto& voice : owned) {
        if (const auto* source = registry.try_get<AudioSourceComponent>(voice.owner)) {
            ma_sound_set_volume(&voice.sound, source->volume);
            ma_sound_set_pitch(&voice.sound, std::max(0.01f, source->pitch));
        }
    }

    if (!registry.ctx().contains<ResourceManager&>()) {
        return;
    }
    auto& resources = registry.ctx().get<ResourceManager&>();

    // Start play-on-create sources exactly once; the tag dies with the component.
    registry.view<AudioSourceComponent>().each([&](auto entity, const AudioSourceComponent& source) {
        if (!source.play_on_create || registry.all_of<AudioStarted>(entity)) {
            return;
        }
        registry.emplace<AudioStarted>(entity);
        const auto* clip = resources.try_get<AudioClip>(source.clip.id);
        if (clip == nullptr) {
            Log::core().warn(std::format("AudioSource references unknown clip '{}'", source.clip.name));
            return;
        }
        impl->play(*clip, source.volume, source.pitch, source.looping, entity);
    });
}

}
