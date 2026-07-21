#include "core/audio.hpp"

#include <algorithm>
#include <format>
#include <list>
#include <stdexcept>

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#include <miniaudio.h>

#include "components/audio.hpp"
#include "core/logger.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"

namespace k2 {

AudioClip::AudioClip(std::span<const std::byte> encoded) {
    ma_decoder decoder;
    auto config = ma_decoder_config_init(ma_format_f32, 0, 0);
    if (ma_decoder_init_memory(encoded.data(), encoded.size(), &config, &decoder) != MA_SUCCESS) {
        throw std::runtime_error("Failed to decode audio data");
    }
    channels = decoder.outputChannels;
    sample_rate = decoder.outputSampleRate;

    std::vector<float> pcm;
    std::vector<float> chunk(std::size_t(4096) * channels);
    ma_uint64 read;
    do {
        read = 0;
        ma_decoder_read_pcm_frames(&decoder, chunk.data(), 4096, &read);
        pcm.insert(pcm.end(), chunk.begin(), chunk.begin() + long(read * channels));
    } while (read == 4096);
    ma_decoder_uninit(&decoder);
    frames = std::make_shared<const std::vector<float>>(std::move(pcm));
}

namespace {
    constexpr std::size_t max_voices = 64;

    struct AudioStarted { };
    struct AudioAttached { };
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
        if (!ready || clip.channels == 0 || !clip.frames || clip.frames->empty() || voices.size() >= max_voices) {
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

    void stop_all() {
        for (auto& voice : voices) {
            release(voice);
        }
        voices.clear();
    }

    void on_source_destroyed(entt::registry& registry, entt::entity entity) {
        registry.remove<AudioStarted>(entity);
        for (auto it = voices.begin(); it != voices.end();) {
            if (it->owner == entity) {
                release(*it);
                it = voices.erase(it);
            } else {
                ++it;
            }
        }
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
    if (!registry.ctx().contains<AudioAttached>()) {
        registry.ctx().emplace<AudioAttached>();
        registry.on_destroy<AudioSourceComponent>().connect<&Impl::on_source_destroyed>(*impl);
    }

    // Reap finished one-shots (no entity, so no destroy signal covers them) and follow
    // component volume/pitch so scripts can change them live.
    for (auto it = impl->voices.begin(); it != impl->voices.end();) {
        bool owner_died = it->owner != entt::null && !registry.valid(it->owner);
        if (owner_died || (!ma_sound_is_looping(&it->sound) && ma_sound_at_end(&it->sound))) {
            impl->release(*it);
            it = impl->voices.erase(it);
        } else {
            if (it->owner != entt::null) {
                if (const auto* source = registry.try_get<AudioSourceComponent>(it->owner)) {
                    ma_sound_set_volume(&it->sound, source->volume);
                    ma_sound_set_pitch(&it->sound, std::max(0.01f, source->pitch));
                }
            }
            ++it;
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
