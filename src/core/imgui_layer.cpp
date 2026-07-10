#include "core/imgui_layer.hpp"

#include "core/logger.hpp"
#include "events/event.hpp"
#include "events/keyboard.hpp"
#include "events/mouse.hpp"
#include "events/window.hpp"
#include "platform/desktop/window_impl.hpp"

#include <IconsFontAwesome5.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <array>

namespace k2 {
ImguiLayer::ImguiLayer(k2::Window& win, std::unique_ptr<Imgui::ImGuiTheme> theme)
    : window(&win)
    , theme(std::move(theme)) {
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
    this->theme->apply();
    if (io.Fonts->AddFontFromFileTTF("res/fonts/NotoSans-Regular.ttf", 20) != nullptr) {
        ImFontConfig config;
        config.MergeMode = true;
        static std::array<const ImWchar, 3> icon_ranges { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromFileTTF("res/fonts/fontawesome-webfont.ttf", 20.0f, &config, icon_ranges.data());
    } else {
        Log::core().warn("Failed to load res/fonts/NotoSans-Regular.ttf, falling back to the default font.");
        io.Fonts->AddFontDefault();
    }

    ImGui_ImplGlfw_InitForOpenGL(glfw_window, false);
    ImGui_ImplOpenGL3_Init();
}

ImguiLayer::~ImguiLayer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized_windows.erase(window);
}

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
    } else if (event->type == "CursorPositionEvent"_fnv1a) {
        auto cursor_event = static_cast<const CursorPositionEvent*>(event);
        ImGui_ImplGlfw_CursorPosCallback(window->impl->window.get(), cursor_event->x, cursor_event->y);
        return ImGui::GetIO().WantCaptureMouse;
    } else if (event->type == "CursorEnterExitEvent"_fnv1a) {
        auto enter_event = static_cast<const CursorEnterExitEvent*>(event);
        ImGui_ImplGlfw_CursorEnterCallback(window->impl->window.get(), enter_event->state);
        return false;
    } else if (event->type == "WindowFocusChangeEvent"_fnv1a) {
        auto focus_event = static_cast<const WindowFocusChangeEvent*>(event);
        ImGui_ImplGlfw_WindowFocusCallback(window->impl->window.get(), focus_event->focused);
        return false;
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
}

}
