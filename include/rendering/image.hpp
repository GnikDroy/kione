#pragma once
#include <concepts>
#include <filesystem>
#include <variant>

#include <stb_image.h>

namespace k2 {

struct Image {
    int width {};
    int height {};
    int channels {};

    template <class T>
        requires std::same_as<T, float> || std::same_as<T, uint8_t>
    using StbiData = std::unique_ptr<T[], void (*)(void*)>;

    using ImageData = std::variant<StbiData<uint8_t>, StbiData<float>>;

    ImageData data = StbiData<uint8_t> { nullptr, stbi_image_free };

    explicit Image() = default;

    explicit Image(const std::filesystem::path& path, int desired_channels = 0) {
        stbi_set_flip_vertically_on_load(1);
        auto is_hdr = stbi_is_hdr(path.string().c_str());
        if (is_hdr) {
            float* ptr = stbi_loadf(path.string().c_str(), &width, &height, &channels, desired_channels);
            data = std::unique_ptr<float[], void (*)(void*)> { ptr, stbi_image_free };
        } else {
            uint8_t* ptr = stbi_load(path.string().c_str(), &width, &height, &channels, desired_channels);
            data = std::unique_ptr<uint8_t[], void (*)(void*)> { ptr, stbi_image_free };
        }
    }

    Image(const std::vector<std::uint8_t>& raw, int desired_channels = 0) {
        stbi_set_flip_vertically_on_load(true);
        auto is_hdr = stbi_is_hdr_from_memory(raw.data(), (int)raw.size());
        if (is_hdr) {
            float* ptr
                = stbi_loadf_from_memory(raw.data(), (int)raw.size(), &width, &height, &channels, desired_channels);
            data = std::unique_ptr<float[], void (*)(void*)> { ptr, stbi_image_free };
        } else {
            uint8_t* ptr
                = stbi_load_from_memory(raw.data(), (int)raw.size(), &width, &height, &channels, desired_channels);
            data = std::unique_ptr<uint8_t[], void (*)(void*)> { ptr, stbi_image_free };
        }
    }

    Image(const Image& image) = delete;
    Image& operator=(const Image& image) = delete;

    Image(Image&& image) noexcept
        : width(image.width)
        , height(image.height)
        , channels(image.channels) {
        std::swap(data, image.data);
    }

    Image& operator=(Image&& image) noexcept {
        std::swap(data, image.data);
        std::swap(width, image.width);
        std::swap(height, image.height);
        std::swap(channels, image.channels);
        return *this;
    }

    static std::string_view error_msg() { return stbi_failure_reason(); }

    operator bool() const {
        return std::visit([](auto& data) { return bool(data); }, data);
    }
};
}
