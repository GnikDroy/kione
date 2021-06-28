#include "core/imgui_layer.hpp"

#include "events/event.hpp"
#include "events/keyboard.hpp"
#include "events/mouse.hpp"
#include "platform/desktop/window_impl.hpp"

#include <IconsFontAwesome5.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <array>
#include <sstream>

// cannot make constexpr since ImVec4 is not constexpr :/
static ImVec4 ImVec4Color(const std::string& hex_code) {
    // Must be of the form #rrggbbaa
    // Might support rgb() and rgba() later
    if (hex_code.starts_with('#')) {
        std::stringstream stream(hex_code);
        stream.ignore(hex_code.length(), '#');
        std::uint32_t color {};
        stream << std::hex;
        stream >> color;
        assert(!stream.fail() && "Cannot interpret color value. Make sure color is of type #rrggbbaa");
        return { (color >> 24) / 255.f, ((color >> 16) & 0x00FF) / 255.f, ((color >> 8) & (0x0000FF)) / 255.f,
            (color & (0x000000FF)) / 255.f };
    }
    assert(false && "Invalid color value.");
    return { 1.0f, 1.0f, 1.0f, 1.0f };
}

static void ImGuiStyleDark() {
    auto& io = ImGui::GetIO();
    auto& style = ImGui::GetStyle();
    auto* colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_Text] = ImVec4Color("#e5e5e5ff");
    colors[ImGuiCol_TextDisabled] = ImVec4Color("#7f7f7fff");
    colors[ImGuiCol_WindowBg] = ImVec4Color("#0c0c0cff");
    colors[ImGuiCol_ChildBg] = ImVec4Color("#0c0c0cff");
    colors[ImGuiCol_PopupBg] = ImVec4Color("#000000e5");
    colors[ImGuiCol_Border] = ImVec4Color("#333333ff");
    colors[ImGuiCol_BorderShadow] = ImVec4Color("#ffffff00");
    colors[ImGuiCol_FrameBg] = ImVec4Color("#000000cc");
    colors[ImGuiCol_FrameBgHovered] = ImVec4Color("#d65e0033");
    colors[ImGuiCol_FrameBgActive] = ImVec4Color("#d65e00ff");
    colors[ImGuiCol_TitleBg] = ImVec4Color("#0f0f0fff");
    colors[ImGuiCol_TitleBgActive] = ImVec4Color("#141414ff");
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4Color("#0f0f0f66");
    colors[ImGuiCol_MenuBarBg] = ImVec4Color("#232323ff");
    colors[ImGuiCol_ScrollbarBg] = ImVec4Color("#23232366");
    colors[ImGuiCol_ScrollbarGrab] = ImVec4Color("#4f4f4f4c");
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4Color("#ffffff4c");
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4Color("#ffffff7f");
    colors[ImGuiCol_CheckMark] = ImVec4Color("#e5e5e5ff");
    colors[ImGuiCol_SliderGrab] = ImVec4Color("#4f4f4fff");
    colors[ImGuiCol_SliderGrabActive] = ImVec4Color("#ffffff7f");
    colors[ImGuiCol_Button] = ImVec4Color("#232323ff");
    colors[ImGuiCol_ButtonHovered] = ImVec4Color("#d65e0033");
    colors[ImGuiCol_ButtonActive] = ImVec4Color("#d65e00ff");
    colors[ImGuiCol_Header] = ImVec4Color("#232323ff");
    colors[ImGuiCol_HeaderHovered] = ImVec4Color("#d65e0033");
    colors[ImGuiCol_HeaderActive] = ImVec4Color("#d65e00ff");
    colors[ImGuiCol_Separator] = ImVec4Color("#7f7f6d7f");
    colors[ImGuiCol_SeparatorHovered] = ImVec4Color("#bf7219c6");
    colors[ImGuiCol_SeparatorActive] = ImVec4Color("#bf7219ff");
    colors[ImGuiCol_ResizeGrip] = ImVec4Color("#f9a5423f");
    colors[ImGuiCol_ResizeGripHovered] = ImVec4Color("#f9a542aa");
    colors[ImGuiCol_ResizeGripActive] = ImVec4Color("#f9a542f2");
    colors[ImGuiCol_Tab] = ImVec4Color("#2b190aef");
    colors[ImGuiCol_TabHovered] = ImVec4Color("#d65e0099");
    colors[ImGuiCol_TabActive] = ImVec4Color("#aa4c00ad");
    colors[ImGuiCol_TabUnfocused] = ImVec4Color("#0f0c0caf");
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4Color("#5b2b07a3");
    colors[ImGuiCol_PlotLines] = ImVec4Color("#636363ff");
    colors[ImGuiCol_PlotLinesHovered] = ImVec4Color("#59eaffff");
    colors[ImGuiCol_PlotHistogram] = ImVec4Color("#0090e5ff");
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4Color("#0066ffff");
    colors[ImGuiCol_TextSelectedBg] = ImVec4Color("#f9a54259");
    colors[ImGuiCol_DragDropTarget] = ImVec4Color("#0090e5ff");
    colors[ImGuiCol_NavHighlight] = ImVec4Color("#f9a542ff");
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4Color("#000000b2");
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4Color("#33333333");
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4Color("#33333359");

    style.WindowPadding = ImVec2(4.f, 6.f);
    style.FramePadding = ImVec2(8.f, 6.f);
    style.ItemSpacing = ImVec2(8.f, 6.f);
    style.ItemInnerSpacing = ImVec2(8.f, 6.f);
    style.IndentSpacing = 20.f;

    style.ScrollbarSize = 20.f;
    style.GrabMinSize = 8.f;
    style.WindowBorderSize = 0.f;
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
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    io.ConfigWindowsMoveFromTitleBarOnly = true;
}

namespace k2 {
ImguiLayer::ImguiLayer(k2::Window& win)
    : window(&win) {
    if (initialized_windows.count(window) != 0) {
        throw std::runtime_error("Imgui layer for this window already exists.");
    }
    initialized_windows.insert(window);

    auto glfw_window = win.impl->window.get();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyleDark();
    io.Fonts->AddFontFromFileTTF("res/fonts/NotoSans-Regular.ttf", 20);

    ImFontConfig config;
    config.MergeMode = true;
    static std::array<const ImWchar, 3> icon_ranges { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromFileTTF("res/fonts/fontawesome-webfont.ttf", 20.0f, &config, icon_ranges.data());

    ImGui_ImplGlfw_InitForOpenGL(glfw_window, false);
    ImGui_ImplOpenGL3_Init();
}

ImguiLayer::~ImguiLayer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized_windows.erase(window);
};

bool ImguiLayer::handle_event(const Event* event) {
    using namespace k2::literals;
    if (event->type == "KeyboardKeyEvent"_fnv1a) {
        auto key_event = static_cast<const KeyboardKeyEvent*>(event);
        ImGui_ImplGlfw_KeyCallback(window->impl->window.get(), static_cast<int>(key_event->code), key_event->scan_code,
            static_cast<int>(key_event->state), static_cast<int>(key_event->mods));
        return ImGui::GetIO().WantCaptureKeyboard;
    } else if (event->type == "KeyboardCharEvent"_fnv1a) {
        auto char_event = static_cast<const KeyboardCharEvent*>(event);
        ImGui_ImplGlfw_CharCallback(window->impl->window.get(), char_event->code);
        return ImGui::GetIO().WantCaptureKeyboard;
    } else if (event->type == "MouseButtonEvent"_fnv1a) {
        auto mouse_event = static_cast<const MouseButtonEvent*>(event);
        ImGui_ImplGlfw_MouseButtonCallback(window->impl->window.get(), static_cast<int>(mouse_event->code),
            static_cast<int>(mouse_event->state), static_cast<int>(mouse_event->mods));
        return ImGui::GetIO().WantCaptureMouse;
    } else if (event->type == "ScrollEvent"_fnv1a) {
        auto scroll_event = static_cast<const ScrollEvent*>(event);
        ImGui_ImplGlfw_ScrollCallback(window->impl->window.get(), scroll_event->x, scroll_event->y);
        return ImGui::GetIO().WantCaptureMouse;
    }
    return false;
}

void ImguiLayer::update(float) { }

void ImguiLayer::start() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImguiLayer::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
#ifdef _WIN32
        auto* context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(context);
#endif
    }
};

}
