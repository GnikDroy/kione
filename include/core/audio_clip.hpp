#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace k2 {

struct AudioClip {
    std::uint32_t channels {};
    std::uint32_t sample_rate {};
    // Shared so playing voices pin the PCM across clip replacement/destruction.
    std::shared_ptr<const std::vector<float>> frames {};

    AudioClip() = default;
    explicit AudioClip(std::span<const std::byte> encoded);
};
}
