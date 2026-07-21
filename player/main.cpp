#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "core/imgui_layer.hpp"
#include "core/paths.hpp"
#include "rendering/debug.hpp"
#include "game_layer.hpp"

#include <algorithm>
#include <filesystem>
#include <format>

class Player : public k2::App {
public:
    explicit Player(const std::string& project_path) {
        k2::Log::app().info("Player application started.");
        k2::enable_debug();

        layers.push_back(std::make_unique<GameLayer>(window, project_path));
        layers.push_back(std::make_unique<k2::ImguiLayer>(window));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
    }

    ~Player() override { k2::Log::app().info("Player application stopped."); }
};

static std::string discover_project() {
    auto directory = k2::executable_path().parent_path();
    std::error_code ec;
    std::vector<std::filesystem::path> projects;
    for (const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
        if (entry.path().extension() == ".k2project") {
            projects.push_back(entry.path());
        }
    }
    if (projects.size() > 1) {
        std::ranges::sort(projects);
        std::string names;
        for (const auto& project : projects) {
            names += std::format("\n  {}", project.filename().string());
        }
        throw std::runtime_error(
            std::format("Multiple projects next to the executable; pass one explicitly:{}", names));
    }
    return projects.front().string();
}

auto create_app(std::vector<std::string> args) -> std::unique_ptr<k2::App> {
    return std::make_unique<Player>(args.empty() ? discover_project() : args.front());
}
