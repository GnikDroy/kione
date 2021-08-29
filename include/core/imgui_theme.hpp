#pragma once
#include "core/fnv.hpp"
#include <imgui.h>
#include <sstream>
#include <string>

namespace k2::Imgui {

// cannot make constexpr since ImVec4 is not constexpr :/
// Must be of the form #rrggbbaa #ggbba #bbaa #aa
inline ImVec4 HexColorToImVec4(const std::string& hex_color_code) {
    if (hex_color_code.starts_with('#')) {
        std::stringstream stream(hex_color_code);
        stream.ignore(std::streamsize(hex_color_code.length()), '#');
        std::uint32_t color {};
        stream << std::hex;
        stream >> color;
        if (stream.fail()) {
            throw std::invalid_argument("Cannot interpret color value. Make sure color is of type #rrggbbaa");
        }
        return { float(color >> 24) / 255, float((color >> 16) & 0x00FF) / 255, float((color >> 8) & (0x0000FF)) / 255,
            float(color & (0x000000FF)) / 255 };
    }
    throw std::invalid_argument("Invalid color value.");
}

struct ImGuiTheme {
    std::unordered_map<std::uint64_t, ImVec4> colors;
    virtual void apply() const = 0;
    virtual ~ImGuiTheme() = default;
};

struct ImGuiThemeDark : ImGuiTheme {

    ImGuiThemeDark() {
        using namespace k2::literals;
        colors["primary"_fnv1a] = HexColorToImVec4("#3F51B5ff");
        colors["primary_alt"_fnv1a] = HexColorToImVec4("#303F9Fff");
        colors["secondary"_fnv1a] = HexColorToImVec4("#792021ff");
        colors["secondary_alt"_fnv1a] = HexColorToImVec4("#7d1021ff");
        colors["background"_fnv1a] = HexColorToImVec4("#181818ff");
        colors["background_alt"_fnv1a] = HexColorToImVec4("#141414ff");

        colors["log_trace"_fnv1a] = HexColorToImVec4("#a0a0a0ff");
        colors["log_debug"_fnv1a] = HexColorToImVec4("#e5e5e5ff");
        colors["log_info"_fnv1a] = HexColorToImVec4("#ffffffff");
        colors["log_warn"_fnv1a] = HexColorToImVec4("#ffff00ff");
        colors["log_err"_fnv1a] = HexColorToImVec4("#aa0000ff");
        colors["log_critical"_fnv1a] = HexColorToImVec4("#ff0000ff");
        colors["log_off"_fnv1a] = colors["background"_fnv1a];
    }

    void apply() const override {
        using namespace k2::literals;
        auto& io = ImGui::GetIO();
        auto& style = ImGui::GetStyle();
        auto* im_colors = ImGui::GetStyle().Colors;

        im_colors[ImGuiCol_Text] = HexColorToImVec4("#e5e5e5ff");
        im_colors[ImGuiCol_TextDisabled] = HexColorToImVec4("#7f7f7fff");
        im_colors[ImGuiCol_WindowBg] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_ChildBg] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_PopupBg] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_Border] = HexColorToImVec4("#262626FF");
        im_colors[ImGuiCol_BorderShadow] = HexColorToImVec4("#ffffff00");
        im_colors[ImGuiCol_FrameBg] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_FrameBgHovered] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_FrameBgActive] = colors.at("primary_alt"_fnv1a);
        im_colors[ImGuiCol_TitleBg] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_TitleBgActive] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_TitleBgCollapsed] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_MenuBarBg] = colors.at("background_alt"_fnv1a);
        im_colors[ImGuiCol_ScrollbarBg] = colors.at("background_alt"_fnv1a);
        im_colors[ImGuiCol_ScrollbarGrab] = HexColorToImVec4("#4f4f4f4c");
        im_colors[ImGuiCol_ScrollbarGrabHovered] = HexColorToImVec4("#ffffff4c");
        im_colors[ImGuiCol_ScrollbarGrabActive] = HexColorToImVec4("#ffffff7f");
        im_colors[ImGuiCol_CheckMark] = HexColorToImVec4("#e5e5e5ff");
        im_colors[ImGuiCol_SliderGrab] = HexColorToImVec4("#4f4f4fff");
        im_colors[ImGuiCol_SliderGrabActive] = HexColorToImVec4("#ffffff7f");
        im_colors[ImGuiCol_Button] = HexColorToImVec4("#232323ff");
        im_colors[ImGuiCol_ButtonHovered] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_ButtonActive] = colors.at("primary_alt"_fnv1a);
        im_colors[ImGuiCol_Header] = HexColorToImVec4("#232323ff");
        im_colors[ImGuiCol_HeaderHovered] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_HeaderActive] = colors.at("primary_alt"_fnv1a);
        im_colors[ImGuiCol_Separator] = HexColorToImVec4("#7f7f6d7f");
        im_colors[ImGuiCol_SeparatorHovered] = colors.at("secondary"_fnv1a);
        im_colors[ImGuiCol_SeparatorActive] = colors.at("secondary_alt"_fnv1a);
        im_colors[ImGuiCol_ResizeGrip] = colors.at("secondary"_fnv1a);
        im_colors[ImGuiCol_ResizeGripHovered] = colors.at("secondary_alt"_fnv1a);
        im_colors[ImGuiCol_ResizeGripActive] = colors.at("secondary_alt"_fnv1a);
        im_colors[ImGuiCol_Tab] = HexColorToImVec4("#262F5DFF");
        im_colors[ImGuiCol_TabHovered] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_TabActive] = colors.at("primary_alt"_fnv1a);
        im_colors[ImGuiCol_TabUnfocused] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_TabUnfocusedActive] = colors.at("primary_alt"_fnv1a);
        im_colors[ImGuiCol_PlotLines] = HexColorToImVec4("#636363ff");
        im_colors[ImGuiCol_PlotLinesHovered] = HexColorToImVec4("#59eaffff");
        im_colors[ImGuiCol_PlotHistogram] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_PlotHistogramHovered] = colors.at("primary_alt"_fnv1a);
        im_colors[ImGuiCol_TextSelectedBg] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_DragDropTarget] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_NavHighlight] = colors.at("primary"_fnv1a);
        im_colors[ImGuiCol_NavWindowingHighlight] = colors.at("background"_fnv1a);
        im_colors[ImGuiCol_NavWindowingDimBg] = HexColorToImVec4("#33333333");
        im_colors[ImGuiCol_ModalWindowDimBg] = HexColorToImVec4("#33333359");

        style.WindowPadding = ImVec2(4.f, 6.f);
        style.FramePadding = ImVec2(8.f, 6.f);
        style.ItemSpacing = ImVec2(8.f, 6.f);
        style.ItemInnerSpacing = ImVec2(8.f, 6.f);
        style.IndentSpacing = 20.f;

        style.ScrollbarSize = 20.f;
        style.GrabMinSize = 8.f;
        style.WindowBorderSize = 1.f;
        style.ChildBorderSize = 0.f;
        style.PopupBorderSize = 1.f;
        style.FrameBorderSize = 1.f;
        style.TabBorderSize = 0.f;

        style.WindowRounding = 5.f;
        style.ChildRounding = 0.f;
        style.FrameRounding = 5.f;
        style.PopupRounding = 5.f;
        style.ScrollbarRounding = 5.f;
        style.GrabRounding = 5.f;
        style.TabRounding = 5.f;

        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.WindowRounding = 0.f;
        style.WindowMenuButtonPosition = ImGuiDir_None;

        io.ConfigWindowsMoveFromTitleBarOnly = true;
    }
};

}