#include "rendering/image.hpp"

#include <stb_image.h>

namespace k2 {

void Image::free_data(void* ptr) { stbi_image_free(ptr); }

Image::Image(const std::filesystem::path& path, int desired_channels) {
    stbi_set_flip_vertically_on_load(1);
    auto is_hdr = stbi_is_hdr(path.string().c_str());
    if (is_hdr) {
        float* ptr = stbi_loadf(path.string().c_str(), &width, &height, &channels, desired_channels);
        data = StbiData<float> { ptr, free_data };
    } else {
        uint8_t* ptr = stbi_load(path.string().c_str(), &width, &height, &channels, desired_channels);
        data = StbiData<uint8_t> { ptr, free_data };
    }
}

Image::Image(const std::vector<std::uint8_t>& raw, int desired_channels) {
    stbi_set_flip_vertically_on_load(true);
    auto is_hdr = stbi_is_hdr_from_memory(raw.data(), (int)raw.size());
    if (is_hdr) {
        float* ptr = stbi_loadf_from_memory(raw.data(), (int)raw.size(), &width, &height, &channels, desired_channels);
        data = StbiData<float> { ptr, free_data };
    } else {
        uint8_t* ptr = stbi_load_from_memory(raw.data(), (int)raw.size(), &width, &height, &channels, desired_channels);
        data = StbiData<uint8_t> { ptr, free_data };
    }
}

std::string_view Image::error_msg() { return stbi_failure_reason(); }

}
