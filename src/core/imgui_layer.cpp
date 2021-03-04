#include "core/imgui_layer.hpp"

#include "imgui_impl_glfw.h"
#include "core/imgui_bgfx/imgui_impl_bgfx.hpp"
#include "bgfx/bgfx.h"
#include "platform/desktop/window_impl.hpp"
#include "GLFW/glfw3.h"

namespace k2 {
    ImguiLayer::ImguiLayer(k2::Window &win) {
        if (initialized_windows.count(window) != 0) {
            throw std::runtime_error("Imgui layer for this window already exists.");
        }
        initialized_windows.insert(window);

        auto glfw_window = win.impl->window.get();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsDark();

        ImGui_Implbgfx_Init(255);
        ImGui_ImplGlfw_InitForOther(glfw_window, true);
    }

    ImguiLayer::~ImguiLayer() {
        ImGui_Implbgfx_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        initialized_windows.erase(window);
    };

    void ImguiLayer::update() {
        ImGui_Implbgfx_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool show_demo = true;
        ImGui::ShowDemoWindow(&show_demo);

        ImGui::Render();
        ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
    };
}
