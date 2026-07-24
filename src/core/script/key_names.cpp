#include "core/script/key_names.hpp"

#include <ranges>
#include <unordered_map>

namespace k2 {
namespace {

    using KeyCode = KeyboardDevice::KeyCode;

    const std::unordered_map<std::string, KeyCode> named_keys {
        { "space", KeyCode::key_space },
        { "apostrophe", KeyCode::key_apostrophe },
        { "comma", KeyCode::key_comma },
        { "minus", KeyCode::key_minus },
        { "period", KeyCode::key_period },
        { "slash", KeyCode::key_slash },
        { "0", KeyCode::key_0 },
        { "1", KeyCode::key_1 },
        { "2", KeyCode::key_2 },
        { "3", KeyCode::key_3 },
        { "4", KeyCode::key_4 },
        { "5", KeyCode::key_5 },
        { "6", KeyCode::key_6 },
        { "7", KeyCode::key_7 },
        { "8", KeyCode::key_8 },
        { "9", KeyCode::key_9 },
        { "semicolon", KeyCode::key_semicolon },
        { "equal", KeyCode::key_equal },
        { "a", KeyCode::key_a },
        { "b", KeyCode::key_b },
        { "c", KeyCode::key_c },
        { "d", KeyCode::key_d },
        { "e", KeyCode::key_e },
        { "f", KeyCode::key_f },
        { "g", KeyCode::key_g },
        { "h", KeyCode::key_h },
        { "i", KeyCode::key_i },
        { "j", KeyCode::key_j },
        { "k", KeyCode::key_k },
        { "l", KeyCode::key_l },
        { "m", KeyCode::key_m },
        { "n", KeyCode::key_n },
        { "o", KeyCode::key_o },
        { "p", KeyCode::key_p },
        { "q", KeyCode::key_q },
        { "r", KeyCode::key_r },
        { "s", KeyCode::key_s },
        { "t", KeyCode::key_t },
        { "u", KeyCode::key_u },
        { "v", KeyCode::key_v },
        { "w", KeyCode::key_w },
        { "x", KeyCode::key_x },
        { "y", KeyCode::key_y },
        { "z", KeyCode::key_z },
        { "left_bracket", KeyCode::key_left_bracket },
        { "backslash", KeyCode::key_backslash },
        { "right_bracket", KeyCode::key_right_bracket },
        { "grave_accent", KeyCode::key_grave_accent },
        { "world_1", KeyCode::key_world_1 },
        { "world_2", KeyCode::key_world_2 },
        { "escape", KeyCode::key_escape },
        { "enter", KeyCode::key_enter },
        { "tab", KeyCode::key_tab },
        { "backspace", KeyCode::key_backspace },
        { "insert", KeyCode::key_insert },
        { "delete", KeyCode::key_delete },
        { "right", KeyCode::key_right },
        { "left", KeyCode::key_left },
        { "down", KeyCode::key_down },
        { "up", KeyCode::key_up },
        { "page_up", KeyCode::key_page_up },
        { "page_down", KeyCode::key_page_down },
        { "home", KeyCode::key_home },
        { "end", KeyCode::key_end },
        { "caps_lock", KeyCode::key_caps_lock },
        { "scroll_lock", KeyCode::key_scroll_lock },
        { "num_lock", KeyCode::key_num_lock },
        { "print_screen", KeyCode::key_print_screen },
        { "pause", KeyCode::key_pause },
        { "f1", KeyCode::key_f1 },
        { "f2", KeyCode::key_f2 },
        { "f3", KeyCode::key_f3 },
        { "f4", KeyCode::key_f4 },
        { "f5", KeyCode::key_f5 },
        { "f6", KeyCode::key_f6 },
        { "f7", KeyCode::key_f7 },
        { "f8", KeyCode::key_f8 },
        { "f9", KeyCode::key_f9 },
        { "f10", KeyCode::key_f10 },
        { "f11", KeyCode::key_f11 },
        { "f12", KeyCode::key_f12 },
        { "f13", KeyCode::key_f13 },
        { "f14", KeyCode::key_f14 },
        { "f15", KeyCode::key_f15 },
        { "f16", KeyCode::key_f16 },
        { "f17", KeyCode::key_f17 },
        { "f18", KeyCode::key_f18 },
        { "f19", KeyCode::key_f19 },
        { "f20", KeyCode::key_f20 },
        { "f21", KeyCode::key_f21 },
        { "f22", KeyCode::key_f22 },
        { "f23", KeyCode::key_f23 },
        { "f24", KeyCode::key_f24 },
        { "f25", KeyCode::key_f25 },
        { "kp_0", KeyCode::key_kp_0 },
        { "kp_1", KeyCode::key_kp_1 },
        { "kp_2", KeyCode::key_kp_2 },
        { "kp_3", KeyCode::key_kp_3 },
        { "kp_4", KeyCode::key_kp_4 },
        { "kp_5", KeyCode::key_kp_5 },
        { "kp_6", KeyCode::key_kp_6 },
        { "kp_7", KeyCode::key_kp_7 },
        { "kp_8", KeyCode::key_kp_8 },
        { "kp_9", KeyCode::key_kp_9 },
        { "kp_decimal", KeyCode::key_kp_decimal },
        { "kp_divide", KeyCode::key_kp_divide },
        { "kp_multiply", KeyCode::key_kp_multiply },
        { "kp_subtract", KeyCode::key_kp_subtract },
        { "kp_add", KeyCode::key_kp_add },
        { "kp_enter", KeyCode::key_kp_enter },
        { "kp_equal", KeyCode::key_kp_equal },
        { "left_shift", KeyCode::key_left_shift },
        { "left_control", KeyCode::key_left_control },
        { "left_alt", KeyCode::key_left_alt },
        { "left_super", KeyCode::key_left_super },
        { "right_shift", KeyCode::key_right_shift },
        { "right_control", KeyCode::key_right_control },
        { "right_alt", KeyCode::key_right_alt },
        { "right_super", KeyCode::key_right_super },
        { "menu", KeyCode::key_menu },
    };

    const std::unordered_map<KeyCode, std::string> key_names = [] {
        std::unordered_map<KeyCode, std::string> reverse;
        for (const auto& [name, code] : named_keys) {
            reverse.emplace(code, name);
        }
        return reverse;
    }();

}

std::optional<KeyboardDevice::KeyCode> key_code_from(const std::string& key) {
    if (auto it = named_keys.find(key); it != named_keys.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string> key_name_from(KeyboardDevice::KeyCode code) {
    if (auto it = key_names.find(code); it != key_names.end()) {
        return it->second;
    }
    return std::nullopt;
}

const char* key_state_name(KeyboardDevice::KeyState state) {
    switch (state) {
    case KeyboardDevice::KeyState::press: return "press";
    case KeyboardDevice::KeyState::release: return "release";
    case KeyboardDevice::KeyState::repeat: return "repeat";
    default: return "unknown";
    }
}

std::vector<std::string> key_names_all() {
    std::vector<std::string> names;
    names.reserve(named_keys.size());
    for (const auto& name : named_keys | std::ranges::views::keys) {
        names.push_back(name);
    }
    return names;
}

}
