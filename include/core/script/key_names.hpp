#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/keyboard.hpp"

namespace k2 {

std::optional<KeyboardDevice::KeyCode> key_code_from(const std::string& key);
std::optional<std::string> key_name_from(KeyboardDevice::KeyCode code);
const char* key_state_name(KeyboardDevice::KeyState state);

std::vector<std::string> key_names_all();

}
