#include "editor_layer.hpp"
#include "core/scene_loader.hpp"
#include <ImGuizmo.h>
#include <format>

namespace k2 {
EditorLayer::EditorLayer(k2::Window& window)
    : k2::ImguiLayer(window) {
    scene.registry.ctx().emplace<EditorLayer&>(*this);
    scene.registry.ctx().emplace<ResourceManager&>(resources);
    resources.set("white", k2::Texture2D::create_white_texture<uint8_t>());

    for (auto& [id, pair] : assets) {
        auto&& [name, asset] = pair;
        k2::Log::core().trace(std::format("Loaded {} asset: '{}'", asset.get_type_strv(), name));
    }

    for (auto& [id, pair] : assets) {
        auto&& [name, asset] = pair;
        if (asset.type == Asset::Type::Image) {
            auto image = AssetLoader::get<k2::Image>(asset);
            resources.set(name, Texture2D { image });
            resources.set(name, std::move(image));
        }
    }
}

void EditorLayer::open_project(const std::filesystem::path& path) {
    auto new_project = Project::load(path);

    for (auto& [id, pair] : new_project.assets) {
        auto& [name, asset] = pair;
        if (asset.type == Asset::Type::Image && !resources.contains<Texture2D>(id)) {
            resources.set(name, Texture2D { AssetLoader::get<Image>(asset) });
        }
    }

    auto new_scene = SceneLoader::load(new_project.main_scene, resources, new_project.assets);
    new_scene.registry.ctx().emplace<EditorLayer&>(*this);
    scene = std::move(new_scene);
    entity_selector.get_widget().reset_selection();
    project = std::move(new_project);
}

void EditorLayer::begin_frame() {
    ImguiLayer::begin_frame();
    ImGuizmo::BeginFrame();
}

void EditorLayer::update(float) {
    ImGui::DockSpaceOverViewport();
    main_menu_widget.render(*this);
    component_inspector.render(*this);
    entity_selector.render(*this);
    log_viewer.render(*this);
    debug_widget.render(*this);
    file_explorer.render(*this);
    viewport2D.render(*this);
}
}
