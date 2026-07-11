#pragma once
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

namespace k2 {

struct Image {
    int width {};
    int height {};
    int channels {};

    static void free_data(void*);

    template <class T>
        requires std::same_as<T, float> || std::same_as<T, uint8_t>
    using StbiData = std::unique_ptr<T[], void (*)(void*)>;

    using ImageData = std::variant<StbiData<uint8_t>, StbiData<float>>;

    ImageData data = StbiData<uint8_t> { nullptr, free_data };

    explicit Image() = default;

    explicit Image(const std::filesystem::path& path, int desired_channels = 0);

    Image(const std::vector<std::uint8_t>& raw, int desired_channels = 0);

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

    static std::string_view error_msg();

    operator bool() const {
        return std::visit([](auto& data) { return bool(data); }, data);
    }
};
}
