#pragma once
#include <filesystem>
#include <stb_image.h>

namespace k2 {
struct Image {
    int width {};
    int height {};
    int channels {};
    std::uint8_t* data {};

    Image(const std::filesystem::path& path, int desired_channels = 0) {
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(path.generic_string().c_str(), &width, &height, &channels, desired_channels);
    }

    Image(const std::vector<std::uint8_t>& raw, int desired_channels = 0) {
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load_from_memory(raw.data(), (int)raw.size(), &width, &height, &channels, desired_channels);
    }

    static std::string_view error_msg() { return stbi_failure_reason(); }

    operator bool() const { return data != nullptr; }

    ~Image() { stbi_image_free(data); }
};
}