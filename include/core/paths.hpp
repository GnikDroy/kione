#pragma once

#include <filesystem>
#include <string>

namespace k2 {

std::filesystem::path executable_path();

void open_url(const std::string& url);

}
