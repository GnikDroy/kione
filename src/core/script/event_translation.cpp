#include "core/script/event_translation.hpp"

#include <format>
#include <string>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/script/key_names.hpp"
#include "events/keyboard.hpp"
#include "events/mouse.hpp"
#include "events/window.hpp"

namespace k2 {
namespace {

    std::string mouse_button_name(MouseDevice::ButtonCode code) {
        switch (int(code)) {
        case int(MouseDevice::ButtonCode::Left): return "left";
        case int(MouseDevice::ButtonCode::Right): return "right";
        case int(MouseDevice::ButtonCode::Middle): return "middle";
        default: return std::format("button_{}", int(code));
        }
    }

    std::string utf8_encode(unsigned int code_point) {
        if (code_point > 0x10FFFF || (code_point >= 0xD800 && code_point <= 0xDFFF)) {
            return "\xEF\xBF\xBD"; // U+FFFD replacement character
        }
        std::string out;
        if (code_point < 0x80) {
            out += char(code_point);
        } else if (code_point < 0x800) {
            out += char(0xC0 | (code_point >> 6));
            out += char(0x80 | (code_point & 0x3F));
        } else if (code_point < 0x10000) {
            out += char(0xE0 | (code_point >> 12));
            out += char(0x80 | ((code_point >> 6) & 0x3F));
            out += char(0x80 | (code_point & 0x3F));
        } else {
            out += char(0xF0 | (code_point >> 18));
            out += char(0x80 | ((code_point >> 12) & 0x3F));
            out += char(0x80 | ((code_point >> 6) & 0x3F));
            out += char(0x80 | (code_point & 0x3F));
        }
        return out;
    }

    void set_mods(sol::table& table, KeyboardDevice::KeyMod mods) {
        auto m = int(mods);
        table["shift"] = (m & int(KeyboardDevice::KeyMod::mod_shift)) != 0;
        table["control"] = (m & int(KeyboardDevice::KeyMod::mod_control)) != 0;
        table["alt"] = (m & int(KeyboardDevice::KeyMod::mod_alt)) != 0;
        table["super"] = (m & int(KeyboardDevice::KeyMod::mod_super)) != 0;
    }

}

std::optional<TranslatedEvent> translate_event(sol::state& lua, const Event* event) {
    auto table = lua.create_table();
    if (event->type == KeyboardKeyEvent::hash) {
        const auto& e = *static_cast<const KeyboardKeyEvent*>(event);
        table["type"] = "key";
        if (auto name = key_name_from(e.code)) {
            table["key"] = *name;
        }
        table["code"] = int(e.code);
        table["scan_code"] = e.scan_code;
        table["state"] = key_state_name(e.state);
        set_mods(table, e.mods);
        return TranslatedEvent { .table = table };
    }
    if (event->type == KeyboardCharEvent::hash) {
        const auto& e = *static_cast<const KeyboardCharEvent*>(event);
        table["type"] = "char";
        table["code"] = e.code;
        table["text"] = utf8_encode(e.code);
        return TranslatedEvent { .table = table };
    }
    if (event->type == MouseButtonEvent::hash) {
        const auto& e = *static_cast<const MouseButtonEvent*>(event);
        table["type"] = "mouse_button";
        table["button"] = mouse_button_name(e.code);
        table["code"] = int(e.code);
        table["state"] = key_state_name(e.state);
        set_mods(table, e.mods);
        return TranslatedEvent { .table = table };
    }
    if (event->type == MouseDropEvent::hash) {
        const auto& e = *static_cast<const MouseDropEvent*>(event);
        table["type"] = "mouse_drop";
        table["paths"] = sol::as_table(e.paths);
        return TranslatedEvent { .table = table };
    }
    if (event->type == CursorPositionEvent::hash) {
        const auto& e = *static_cast<const CursorPositionEvent*>(event);
        table["type"] = "cursor_position";
        table["x"] = e.x;
        table["y"] = e.y;
        return TranslatedEvent { .table = table };
    }
    if (event->type == CursorEnterExitEvent::hash) {
        const auto& e = *static_cast<const CursorEnterExitEvent*>(event);
        table["type"] = "cursor_enter";
        table["entered"] = e.state;
        return TranslatedEvent { .table = table };
    }
    if (event->type == ScrollEvent::hash) {
        const auto& e = *static_cast<const ScrollEvent*>(event);
        table["type"] = "scroll";
        table["x"] = e.x;
        table["y"] = e.y;
        return TranslatedEvent { .table = table };
    }
    if (event->type == WindowCloseEvent::hash) {
        table["type"] = "window_close";
        return TranslatedEvent { .table = table, .input = false };
    }
    if (event->type == WindowResizeEvent::hash) {
        const auto& e = *static_cast<const WindowResizeEvent*>(event);
        table["type"] = "window_resize";
        table["width"] = e.width;
        table["height"] = e.height;
        return TranslatedEvent { .table = table, .input = false };
    }
    if (event->type == WindowFramebufferResizeEvent::hash) {
        const auto& e = *static_cast<const WindowFramebufferResizeEvent*>(event);
        table["type"] = "framebuffer_resize";
        table["width"] = e.width;
        table["height"] = e.height;
        return TranslatedEvent { .table = table, .input = false };
    }
    if (event->type == WindowContentScaleChangeEvent::hash) {
        const auto& e = *static_cast<const WindowContentScaleChangeEvent*>(event);
        table["type"] = "content_scale";
        table["x"] = e.x;
        table["y"] = e.y;
        return TranslatedEvent { .table = table, .input = false };
    }
    if (event->type == WindowRepositionEvent::hash) {
        const auto& e = *static_cast<const WindowRepositionEvent*>(event);
        table["type"] = "window_reposition";
        table["x"] = e.x;
        table["y"] = e.y;
        return TranslatedEvent { .table = table, .input = false };
    }
    if (event->type == WindowIconifyEvent::hash) {
        const auto& e = *static_cast<const WindowIconifyEvent*>(event);
        table["type"] = "window_iconify";
        table["iconified"] = e.iconified;
        return TranslatedEvent { .table = table, .input = false };
    }
    if (event->type == WindowMaximizeEvent::hash) {
        const auto& e = *static_cast<const WindowMaximizeEvent*>(event);
        table["type"] = "window_maximize";
        table["maximized"] = e.maximized;
        return TranslatedEvent { .table = table, .input = false };
    }
    if (event->type == WindowFocusChangeEvent::hash) {
        const auto& e = *static_cast<const WindowFocusChangeEvent*>(event);
        table["type"] = "window_focus";
        table["focused"] = e.focused;
        return TranslatedEvent { .table = table, .input = false };
    }
    return std::nullopt;
}

}
