#pragma once
#include "stb_image.h"
#include <filesystem>

namespace k2 {
struct Image {
    int width {};
    int height {};
    int channels {};
    stbi_uc* data {};

    explicit Image(const std::filesystem::path& path, int desired_channels = 0) {
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(path.generic_string().c_str(), &width, &height, &channels, desired_channels);
    }

    static std::string_view error_msg() { return stbi_failure_reason(); }

    operator bool() const { return data != nullptr; }

    ~Image() { stbi_image_free(data); }
};
}