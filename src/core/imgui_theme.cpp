#include "core/imgui_theme.hpp"

#include <charconv>
#include <stdexcept>
#include <string_view>

#include "core/fnv.hpp"

namespace k2::Imgui {

ImVec4 HexColorToImVec4(const std::string& hex_color_code) {
    if (!hex_color_code.starts_with('#')) {
        throw std::invalid_argument("Color must start with '#'.");
    }
    std::string_view digits { hex_color_code };
    digits.remove_prefix(1);
    if (digits.size() != 6 && digits.size() != 8) {
        throw std::invalid_argument("Color must be of the form #rrggbb or #rrggbbaa.");
    }
    std::uint32_t value {};
    auto [ptr, ec] = std::from_chars(digits.data(), digits.data() + digits.size(), value, 16);
    if (ec != std::errc {} || ptr != digits.data() + digits.size()) {
        throw std::invalid_argument("Cannot interpret color value. Make sure color is of type #rrggbb or #rrggbbaa.");
    }
    if (digits.size() == 6) {
        value = (value << 8) | 0xff; // implicit opaque alpha
    }
    return { float((value >> 24) & 0xff) / 255.0f, float((value >> 16) & 0xff) / 255.0f,
        float((value >> 8) & 0xff) / 255.0f, float(value & 0xff) / 255.0f };
}

ImGuiThemeDark::ImGuiThemeDark() {
    using namespace k2::literals;
    colors["primary"_fnv1a] = HexColorToImVec4("#5C6BC0ff");
    colors["primary_alt"_fnv1a] = HexColorToImVec4("#3F51B5ff");
    colors["secondary"_fnv1a] = HexColorToImVec4("#ddddddff");
    colors["secondary_alt"_fnv1a] = HexColorToImVec4("#ccccccff");
    colors["background"_fnv1a] = HexColorToImVec4("#1a1a1eff");
    colors["background_alt"_fnv1a] = HexColorToImVec4("#121214ff");
    colors["surface"_fnv1a] = HexColorToImVec4("#26262bff");
    colors["surface_hover"_fnv1a] = HexColorToImVec4("#32323aff");
    colors["surface_active"_fnv1a] = HexColorToImVec4("#3a3a44ff");

    colors["log_trace"_fnv1a] = HexColorToImVec4("#8b8b8bff");
    colors["log_debug"_fnv1a] = HexColorToImVec4("#b8b8b8ff");
    colors["log_info"_fnv1a] = HexColorToImVec4("#e8e8eaff");
    colors["log_warn"_fnv1a] = HexColorToImVec4("#e5c07bff");
    colors["log_err"_fnv1a] = HexColorToImVec4("#e06c75ff");
    colors["log_critical"_fnv1a] = HexColorToImVec4("#ff4d4dff");
    colors["log_off"_fnv1a] = colors["background"_fnv1a];
}

void ImGuiThemeDark::apply() const {
    using namespace k2::literals;
    auto& io = ImGui::GetIO();
    auto& style = ImGui::GetStyle();
    auto* im_colors = ImGui::GetStyle().Colors;

    im_colors[ImGuiCol_Text] = HexColorToImVec4("#e8e8eaff");
    im_colors[ImGuiCol_TextDisabled] = HexColorToImVec4("#70707aff");
    im_colors[ImGuiCol_WindowBg] = colors.at("background"_fnv1a);
    im_colors[ImGuiCol_ChildBg] = HexColorToImVec4("#00000000");
    im_colors[ImGuiCol_PopupBg] = HexColorToImVec4("#202025fa");
    im_colors[ImGuiCol_Border] = HexColorToImVec4("#2a2a30ff");
    im_colors[ImGuiCol_BorderShadow] = HexColorToImVec4("#00000000");
    im_colors[ImGuiCol_FrameBg] = colors.at("surface"_fnv1a);
    im_colors[ImGuiCol_FrameBgHovered] = colors.at("surface_hover"_fnv1a);
    im_colors[ImGuiCol_FrameBgActive] = colors.at("surface_active"_fnv1a);
    im_colors[ImGuiCol_TitleBg] = colors.at("background_alt"_fnv1a);
    im_colors[ImGuiCol_TitleBgActive] = colors.at("background_alt"_fnv1a);
    im_colors[ImGuiCol_TitleBgCollapsed] = colors.at("background_alt"_fnv1a);
    im_colors[ImGuiCol_MenuBarBg] = colors.at("background_alt"_fnv1a);
    im_colors[ImGuiCol_ScrollbarBg] = HexColorToImVec4("#00000000");
    im_colors[ImGuiCol_ScrollbarGrab] = HexColorToImVec4("#3a3a42ff");
    im_colors[ImGuiCol_ScrollbarGrabHovered] = HexColorToImVec4("#4a4a54ff");
    im_colors[ImGuiCol_ScrollbarGrabActive] = colors.at("primary"_fnv1a);
    im_colors[ImGuiCol_CheckMark] = colors.at("primary"_fnv1a);
    im_colors[ImGuiCol_SliderGrab] = HexColorToImVec4("#5C6BC0cc");
    im_colors[ImGuiCol_SliderGrabActive] = HexColorToImVec4("#7986cbff");
    im_colors[ImGuiCol_Button] = colors.at("surface"_fnv1a);
    im_colors[ImGuiCol_ButtonHovered] = colors.at("surface_hover"_fnv1a);
    im_colors[ImGuiCol_ButtonActive] = colors.at("primary_alt"_fnv1a);
    im_colors[ImGuiCol_Header] = HexColorToImVec4("#3F51B566");
    im_colors[ImGuiCol_HeaderHovered] = HexColorToImVec4("#5C6BC04d");
    im_colors[ImGuiCol_HeaderActive] = HexColorToImVec4("#3F51B599");
    im_colors[ImGuiCol_Separator] = HexColorToImVec4("#2a2a30ff");
    im_colors[ImGuiCol_SeparatorHovered] = colors.at("primary"_fnv1a);
    im_colors[ImGuiCol_SeparatorActive] = HexColorToImVec4("#7986cbff");
    im_colors[ImGuiCol_ResizeGrip] = HexColorToImVec4("#00000000");
    im_colors[ImGuiCol_ResizeGripHovered] = HexColorToImVec4("#5C6BC066");
    im_colors[ImGuiCol_ResizeGripActive] = HexColorToImVec4("#5C6BC0cc");
    im_colors[ImGuiCol_Tab] = colors.at("background_alt"_fnv1a);
    im_colors[ImGuiCol_TabHovered] = HexColorToImVec4("#3F51B580");
    im_colors[ImGuiCol_TabActive] = HexColorToImVec4("#2b2f55ff");
    im_colors[ImGuiCol_TabUnfocused] = colors.at("background_alt"_fnv1a);
    im_colors[ImGuiCol_TabUnfocusedActive] = HexColorToImVec4("#202024ff");
    im_colors[ImGuiCol_DockingPreview] = HexColorToImVec4("#5C6BC066");
    im_colors[ImGuiCol_DockingEmptyBg] = colors.at("background_alt"_fnv1a);
    im_colors[ImGuiCol_TableHeaderBg] = HexColorToImVec4("#202024ff");
    im_colors[ImGuiCol_TableBorderStrong] = HexColorToImVec4("#2a2a30ff");
    im_colors[ImGuiCol_TableBorderLight] = HexColorToImVec4("#222227ff");
    im_colors[ImGuiCol_TableRowBg] = HexColorToImVec4("#00000000");
    im_colors[ImGuiCol_TableRowBgAlt] = HexColorToImVec4("#ffffff05");
    im_colors[ImGuiCol_PlotLines] = HexColorToImVec4("#6a6a74ff");
    im_colors[ImGuiCol_PlotLinesHovered] = HexColorToImVec4("#59eaffff");
    im_colors[ImGuiCol_PlotHistogram] = colors.at("primary"_fnv1a);
    im_colors[ImGuiCol_PlotHistogramHovered] = HexColorToImVec4("#7986cbff");
    im_colors[ImGuiCol_TextSelectedBg] = HexColorToImVec4("#3F51B566");
    im_colors[ImGuiCol_DragDropTarget] = HexColorToImVec4("#7986cbff");
    im_colors[ImGuiCol_NavHighlight] = colors.at("primary"_fnv1a);
    im_colors[ImGuiCol_NavWindowingHighlight] = colors.at("background"_fnv1a);
    im_colors[ImGuiCol_NavWindowingDimBg] = HexColorToImVec4("#00000059");
    im_colors[ImGuiCol_ModalWindowDimBg] = HexColorToImVec4("#00000073");

    style.WindowPadding = ImVec2(10.f, 10.f);
    style.FramePadding = ImVec2(8.f, 4.f);
    style.ItemSpacing = ImVec2(8.f, 5.f);
    style.ItemInnerSpacing = ImVec2(6.f, 4.f);
    style.IndentSpacing = 20.f;
    style.CellPadding = ImVec2(6.f, 4.f);

    style.ScrollbarSize = 12.f;
    style.GrabMinSize = 10.f;
    style.WindowBorderSize = 1.f;
    style.ChildBorderSize = 0.f;
    style.PopupBorderSize = 1.f;
    style.FrameBorderSize = 0.f;
    style.TabBorderSize = 0.f;

    style.WindowRounding = 0.f;
    style.ChildRounding = 0.f;
    style.FrameRounding = 4.f;
    style.PopupRounding = 4.f;
    style.ScrollbarRounding = 9.f;
    style.GrabRounding = 4.f;
    style.TabRounding = 4.f;

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;

    io.ConfigWindowsMoveFromTitleBarOnly = true;
}

}
