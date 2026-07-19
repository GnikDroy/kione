#include <catch2/catch_all.hpp>

#include "core/audio.hpp"

#include <cstddef>
#include <cstdint>
#include <numbers>
#include <vector>

namespace {

void put_u32(std::vector<std::byte>& out, std::uint32_t value) {
    for (int i = 0; i < 4; i++) {
        out.push_back(std::byte((value >> (i * 8)) & 0xFF));
    }
}

void put_u16(std::vector<std::byte>& out, std::uint16_t value) {
    out.push_back(std::byte(value & 0xFF));
    out.push_back(std::byte(value >> 8));
}

void put_tag(std::vector<std::byte>& out, const char (&tag)[5]) {
    for (int i = 0; i < 4; i++) {
        out.push_back(std::byte(tag[i]));
    }
}

// A minimal mono 16-bit PCM WAV containing a sine burst.
std::vector<std::byte> make_wav(std::uint32_t sample_rate, std::uint32_t frame_count) {
    std::vector<std::byte> out;
    std::uint32_t data_size = frame_count * 2;
    put_tag(out, "RIFF");
    put_u32(out, 36 + data_size);
    put_tag(out, "WAVE");
    put_tag(out, "fmt ");
    put_u32(out, 16);
    put_u16(out, 1); // PCM
    put_u16(out, 1); // mono
    put_u32(out, sample_rate);
    put_u32(out, sample_rate * 2); // byte rate
    put_u16(out, 2); // block align
    put_u16(out, 16); // bits per sample
    put_tag(out, "data");
    put_u32(out, data_size);
    for (std::uint32_t i = 0; i < frame_count; i++) {
        auto sample = std::int16_t(20000.0 * std::sin(2.0 * std::numbers::pi * 440.0 * i / sample_rate));
        put_u16(out, std::uint16_t(sample));
    }
    return out;
}

}

TEST_CASE("AudioClip decodes a WAV into interleaved f32 PCM") {
    auto wav = make_wav(22050, 1000);

    k2::AudioClip clip { wav };

    REQUIRE(clip.channels == 1);
    REQUIRE(clip.sample_rate == 22050);
    REQUIRE(clip.frames.size() == 1000);
    auto peak = *std::ranges::max_element(clip.frames);
    REQUIRE(peak > 0.5f); // the sine actually made it through
    REQUIRE(peak <= 1.0f);
}

TEST_CASE("AudioClip rejects a non-audio blob") {
    std::vector<std::byte> garbage(256, std::byte { 7 });
    REQUIRE_THROWS(k2::AudioClip { garbage });
}
