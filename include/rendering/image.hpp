#pragma once
#include <filesystem>
#include <stb_image.h>

namespace k2 {
struct Image {
    int width {};
    int height {};
    int channels {};
    std::uint8_t* data {};

    explicit Image() = default;

    explicit Image(const std::filesystem::path& path, int desired_channels = 0) {
        stbi_set_flip_vertically_on_load(1);
        data = stbi_load(path.generic_string().c_str(), &width, &height, &channels, desired_channels);
    }

    Image(const std::vector<std::uint8_t>& raw, int desired_channels = 0) {
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load_from_memory(raw.data(), (int)raw.size(), &width, &height, &channels, desired_channels);
    }

    Image(const Image& image) = delete;
    Image& operator=(const Image& image) = delete;

    Image(Image&& image) noexcept
        : width(image.width)
        , height(image.height)
        , channels(image.channels) {
        std::swap(data, image.data);
    }

    Image& operator=(Image&& image) {
        std::swap(data, image.data);
        std::swap(width, image.width);
        std::swap(height, image.height);
        std::swap(channels, image.channels);
        return *this;
    }

    static std::string_view error_msg() { return stbi_failure_reason(); }

    operator bool() const { return data != nullptr; }

    ~Image() { stbi_image_free(data); }
};
}