#include "core/audio_clip.hpp"

#include <stdexcept>

#include "core/miniaudio.hpp"

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

}
