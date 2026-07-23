#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include <imgui.h>

namespace k2::Imgui {

// Must be of the form #rrggbb or #rrggbbaa.
ImVec4 HexColorToImVec4(const std::string& hex_color_code);

struct ImGuiTheme {
    std::unordered_map<std::uint64_t, ImVec4> colors;
    virtual void apply() const = 0;
    virtual ~ImGuiTheme() = default;

    [[nodiscard]] ImVec4 color(std::string_view name) const;
};

struct ImGuiThemeDark : ImGuiTheme {
    ImGuiThemeDark();

    void apply() const override;
};

struct ImGuiThemeLight : ImGuiTheme {
    ImGuiThemeLight();

    void apply() const override;
};

}
