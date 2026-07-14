#include "editor_layer.hpp"
#include "asset/loader.hpp"
#include "core/animation_system.hpp"
#include "core/scene_loader.hpp"
#include "core/window.hpp"
#include "events/window.hpp"
#include "serializers/core/scene.hpp" // IWYU pragma: keep
#include <ImGuizmo.h>
#include <format>
#include <fstream>
#include <imgui_internal.h>

namespace k2 {
EditorLayer::EditorLayer(k2::Window& window)
    : k2::ImguiLayer(window)
    , scripts { window } {
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

void EditorLayer::load_image_resources(const AssetRegistry& asset_registry) {
    for (auto& [id, pair] : asset_registry) {
        auto& [name, asset] = pair;
        if (asset.type == Asset::Type::Image && !resources.contains<Texture2D>(id)) {
            resources.set(name, Texture2D { AssetLoader::get<Image>(asset) });
        }
    }
}

void EditorLayer::reload_assets() {
    if (!project.has_value()) {
        return;
    }
    project->reload_assets();
    load_image_resources(project->assets);
}

void EditorLayer::open_project(const std::filesystem::path& path) {
    auto new_project = Project::load(path);
    load_image_resources(new_project.assets);

    auto new_scene = SceneLoader::load(new_project.main_scene, resources, new_project.assets);
    new_scene.registry.ctx().emplace<EditorLayer&>(*this);
    scene = std::move(new_scene);
    entity_selector.get_widget().reset_selection();
    project = std::move(new_project);
}

void EditorLayer::create_project(const std::filesystem::path& path) {
    auto project_file = path;
    if (project_file.extension() != ".k2project") {
        project_file += ".k2project";
    }

    Project new_project;
    new_project.file = std::filesystem::absolute(project_file);
    new_project.root = new_project.file.parent_path();
    new_project.name = project_file.stem().string();
    new_project.main_scene = new_project.root / (new_project.name + ".k2scene");

    std::ofstream { new_project.main_scene } << YAML::Node { Scene {} } << "\n";
    new_project.save();

    open_project(project_file);
}

void EditorLayer::request_exit() { window->events.push(std::make_unique<WindowCloseEvent>()); }

void EditorLayer::play() {
    try {
        auto copy = SceneLoader::load(YAML::Node { scene }, resources, active_assets());
        copy.registry.ctx().emplace<EditorLayer&>(*this);
        runtime_scene = std::move(copy);
        scripts.clear_cache();
        entity_selector.get_widget().reset_selection();
    } catch (const std::exception& e) {
        Log::core().error(std::format("Failed to start play mode: {}", e.what()));
    }
}

void EditorLayer::stop() {
    runtime_scene.reset();
    entity_selector.get_widget().reset_selection();
}

void EditorLayer::begin_frame() {
    ImguiLayer::begin_frame();
    ImGuizmo::BeginFrame();
}

void EditorLayer::build_default_layout(unsigned int dockspace_id) {
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    ImGuiID center = dockspace_id;
    ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.20f, nullptr, &center);
    ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
    ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.30f, nullptr, &center);

    ImGui::DockBuilderDockWindow(entity_selector.title.c_str(), left);
    ImGui::DockBuilderDockWindow(component_inspector.title.c_str(), right);
    ImGui::DockBuilderDockWindow(project_settings.title.c_str(), right);
    ImGui::DockBuilderDockWindow(log_viewer.title.c_str(), bottom);
    ImGui::DockBuilderDockWindow(file_explorer.title.c_str(), bottom);
    ImGui::DockBuilderDockWindow(asset_list.title.c_str(), bottom);
    ImGui::DockBuilderDockWindow(debug_widget.title.c_str(), bottom);
    ImGui::DockBuilderDockWindow(viewport2D.title.c_str(), center);
    ImGui::DockBuilderFinish(dockspace_id);
}

void EditorLayer::update(float dt) {
    if (runtime_scene) {
        scripts.update(*runtime_scene, active_assets(), dt);
        AnimationSystem::update(*runtime_scene, dt);
    }
    auto dockspace_id = ImGui::DockSpaceOverViewport();
    if (layout_pending) {
        layout_pending = false;
        build_default_layout(dockspace_id);
    }
    main_menu_widget.render(*this);
    component_inspector.render(*this);
    entity_selector.render(*this);
    project_settings.render(*this);
    asset_list.render(*this);
    log_viewer.render(*this);
    debug_widget.render(*this);
    file_explorer.render(*this);
    viewport2D.render(*this);
}
}
