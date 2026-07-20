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
        if (asset.type != Asset::Type::Image || resources.contains<Texture2D>(id)) {
            continue;
        }
        auto image = AssetLoader::try_get<Image>(asset);
        if (!image) {
            Log::core().error(std::format("Failed to load image '{}': {}", name, image.error()));
            continue;
        }
        resources.set(name, Texture2D { *image });
    }
}

std::expected<void, std::string> EditorLayer::reload_assets() {
    if (!project.has_value()) {
        return {};
    }
    if (auto reloaded = project->reload_assets(); !reloaded) {
        return reloaded;
    }
    load_image_resources(project->assets);
    return {};
}

std::expected<void, std::string> EditorLayer::open_project(const std::filesystem::path& path) {
    auto new_project = Project::load(path);
    if (!new_project) {
        return std::unexpected(new_project.error());
    }
    load_image_resources(new_project->assets);

    auto new_scene = SceneLoader::load(new_project->main_scene, resources, new_project->assets);
    if (!new_scene) {
        return std::unexpected(new_scene.error());
    }
    new_scene->registry.ctx().emplace<EditorLayer&>(*this);
    scene = std::move(*new_scene);
    current_scene = new_project->main_scene;
    entity_selector.get_widget().reset_selection();
    project = std::move(*new_project);
    return {};
}

std::expected<void, std::string> EditorLayer::open_scene(std::string_view name) {
    auto new_scene = SceneLoader::load(name, resources, active_assets());
    if (!new_scene) {
        return std::unexpected(new_scene.error());
    }
    new_scene->registry.ctx().emplace<EditorLayer&>(*this);
    scene = std::move(*new_scene);
    current_scene = name;
    entity_selector.get_widget().reset_selection();
    return {};
}

std::expected<void, std::string> EditorLayer::open_scene_file(const std::filesystem::path& path) {
    std::error_code ec;
    auto target = std::filesystem::weakly_canonical(path, ec);
    for (const auto& [id, pair] : active_assets()) {
        const auto& [name, asset] = pair;
        if (asset.type != Asset::Type::Scene) {
            continue;
        }
        auto asset_path = std::filesystem::weakly_canonical(
            std::filesystem::path { std::string { asset.get_url_divisions().path } }, ec);
        if (asset_path == target) {
            return open_scene(name);
        }
    }
    return std::unexpected(std::format("'{}' is not a Scene asset of the open project", path.filename().string()));
}

std::expected<void, std::string> EditorLayer::create_scene(const std::filesystem::path& path) {
    if (!project.has_value()) {
        return std::unexpected("No project is open.");
    }
    auto scene_file = path;
    if (scene_file.extension() != ".k2scene") {
        scene_file += ".k2scene";
    }
    auto name = scene_file.stem().string();

    std::ofstream scene_out { scene_file };
    scene_out << YAML::Node { Scene {} } << "\n";
    if (!scene_out) {
        return std::unexpected(std::format("Failed to write scene file: {}", scene_file.string()));
    }

    if (auto added = project->add_asset(Asset::Type::Scene, name, scene_file); !added) {
        return added;
    }
    return open_scene(name);
}

std::expected<void, std::string> EditorLayer::create_project(const std::filesystem::path& path) {
    auto project_file = path;
    if (project_file.extension() != ".k2project") {
        project_file += ".k2project";
    }

    Project new_project;
    new_project.file = std::filesystem::absolute(project_file);
    new_project.root = new_project.file.parent_path();
    new_project.name = project_file.stem().string();
    new_project.main_scene = new_project.name;
    auto scene_file = new_project.root / (new_project.name + ".k2scene");

    std::ofstream scene_out { scene_file };
    scene_out << YAML::Node { Scene {} } << "\n";
    if (!scene_out) {
        return std::unexpected(std::format("Failed to write scene file: {}", scene_file.string()));
    }
    YAML::Node assets_node { YAML::NodeType::Map };
    assets_node["Scene"][new_project.name] = std::format("file:///{}.k2scene", new_project.name);
    new_project.assets_node = assets_node;
    if (auto saved = new_project.save(); !saved) {
        return saved;
    }

    return open_project(project_file);
}

void EditorLayer::request_exit() { window->events.push(std::make_unique<WindowCloseEvent>()); }

void EditorLayer::play() {
    auto copy = SceneLoader::load(YAML::Node { scene }, resources, active_assets());
    if (!copy) {
        Log::core().error(std::format("Failed to start play mode: {}", copy.error()));
        return;
    }
    copy->registry.ctx().emplace<EditorLayer&>(*this);
    copy->registry.ctx().emplace<AudioSystem&>(audio);
    runtime_scene = std::move(*copy);
    scripts.clear_cache();
    entity_selector.get_widget().reset_selection();
}

void EditorLayer::stop() {
    audio.stop_all();
    runtime_scene.reset();
    entity_selector.get_widget().reset_selection();
}

void EditorLayer::begin_frame() {
    ImguiLayer::begin_frame();
    ImGuizmo::BeginFrame();
}

bool EditorLayer::handle_event(const Event* event) {
    // ImGui must always see the event to keep its state coherent.
    bool imgui_wants = ImguiLayer::handle_event(event);
    if (runtime_scene && scripts.handle_event(*runtime_scene, active_assets(), event)) {
        return true;
    }
    return imgui_wants;
}

void EditorLayer::build_default_layout(unsigned int dockspace_id) {
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    ImGuiID center = dockspace_id;
    ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.20f, nullptr, &center);
    ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
    ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.30f, nullptr, &center);
    ImGuiID left_bottom = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.30f, nullptr, &left);
    ImGuiID right_bottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.30f, nullptr, &right);

    ImGui::DockBuilderDockWindow(entity_selector.title.c_str(), left);
    ImGui::DockBuilderDockWindow(asset_list.title.c_str(), left_bottom);
    ImGui::DockBuilderDockWindow(component_inspector.title.c_str(), right);
    ImGui::DockBuilderDockWindow(project_settings.title.c_str(), right);
    ImGui::DockBuilderDockWindow(animation_editor.title.c_str(), right_bottom);

    ImGui::DockBuilderDockWindow(log_viewer.title.c_str(), bottom);
    ImGui::DockBuilderDockWindow(file_explorer.title.c_str(), bottom);
    ImGui::DockBuilderDockWindow(debug_widget.title.c_str(), bottom);
    ImGui::DockBuilderDockWindow(viewport2D.title.c_str(), center);
    ImGui::DockBuilderFinish(dockspace_id);
}

void EditorLayer::fixed_update(float dt) {
    if (runtime_scene) {
        scripts.fixed_update(*runtime_scene, active_assets(), dt);
    }
}

void EditorLayer::update(float dt) {
    if (runtime_scene) {
        scripts.update(*runtime_scene, active_assets(), dt);
        AnimationSystem::update(*runtime_scene, dt);
        audio.update(*runtime_scene);
        if (const auto* request = runtime_scene->registry.ctx().find<SceneRequest>()) {
            auto loaded = SceneLoader::load(request->scene, resources, active_assets());
            if (loaded) {
                loaded->registry.ctx().emplace<EditorLayer&>(*this);
                loaded->registry.ctx().emplace<AudioSystem&>(audio);
                runtime_scene = std::move(*loaded);
                scripts.clear_cache();
                audio.stop_all();
                entity_selector.get_widget().reset_selection();
            } else {
                Log::core().error(std::format("Scene switch failed: {}", loaded.error()));
                runtime_scene->registry.ctx().erase<SceneRequest>();
            }
        }
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
    animation_editor.render(*this);
    log_viewer.render(*this);
    debug_widget.render(*this);
    file_explorer.render(*this);
    viewport2D.render(*this);
}
}
