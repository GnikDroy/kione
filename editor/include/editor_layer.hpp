#include "asset/asset_registry.hpp"
#include "core/imgui_layer.hpp"
#include "core/project.hpp"
#include "core/runtime.hpp"
#include "core/scene.hpp"

#include <expected>
#include <filesystem>
#include <optional>

#include "ui/widgets/main_menu.hpp"
#include "ui/windows/animation_editor.hpp"
#include "ui/windows/asset_list.hpp"
#include "ui/windows/component_inspector.hpp"
#include "ui/windows/entity_selector.hpp"
#include "ui/windows/file_explorer.hpp"
#include "ui/windows/log_viewer.hpp"
#include "ui/windows/project_settings.hpp"
#include "ui/windows/tilemap_editor.hpp"
#include "ui/windows/tileset_editor.hpp"
#include "ui/windows/viewport2D.hpp"

namespace k2 {
class EditorLayer : public k2::ImguiLayer {
public:
    AssetRegistry assets
        = AssetRegistryLoader::load({ .url = "file:///res/editor.yaml", .type = Asset::Type::AssetBundle });

    Runtime runtime;
    std::optional<Project> project;

    Scene scene;
    std::string current_scene;
    std::optional<Scene> runtime_scene;

    k2::editor::MainMenuWidget main_menu_widget;
    k2::editor::ComponentInspectorWindow<entt::entity> component_inspector { ICON_MS_BUILD "  Inspector" };
    k2::editor::EntitySelectorWindow<entt::entity> entity_selector { ICON_MS_ACCOUNT_TREE "  Entity Selector" };
    k2::editor::LogViewerWindow log_viewer { ICON_MS_ARTICLE "  Log Viewer" };
    k2::editor::FileExplorerWindow file_explorer { ICON_MS_DESCRIPTION "  File Explorer" };
    k2::editor::Viewport2DWindow viewport2D { ICON_MS_VISIBILITY "  Viewport 2D" };
    k2::editor::ProjectSettingsWindow project_settings { ICON_MS_SETTINGS "  Project Settings" };
    k2::editor::AssetListWindow asset_list { ICON_MS_INVENTORY_2 "  Assets" };
    k2::editor::AnimationEditorWindow animation_editor { ICON_MS_MOVIE "  Animation" };
    k2::editor::TileSetEditorWindow tileset_editor { ICON_MS_BRUSH "  Tile Set" };
    k2::editor::TileMapEditorWindow tilemap_editor { ICON_MS_GRID_ON "  Tilemap Editor" };

    explicit EditorLayer(k2::Window& window);
    void begin_frame() override;
    void fixed_update(float) override;
    void update(float) override;
    bool handle_event(const Event* event) override;

private:
    void build_default_layout(unsigned int dockspace_id);

    bool layout_pending = !std::filesystem::exists("imgui.ini");

public:
    [[nodiscard]] std::expected<void, std::string> open_project(const std::filesystem::path& path);

    [[nodiscard]] std::expected<void, std::string> open_scene(std::string_view name);

    [[nodiscard]] std::expected<void, std::string> open_scene_file(const std::filesystem::path& path);

    [[nodiscard]] std::expected<void, std::string> create_scene(const std::filesystem::path& path);

    [[nodiscard]] std::expected<void, std::string> create_project(const std::filesystem::path& path);

    [[nodiscard]] std::expected<void, std::string> reload_assets();

    void request_exit();

    void reset_layout() { layout_pending = true; }

    void play();
    void stop();
    [[nodiscard]] bool is_playing() const { return runtime_scene.has_value(); }
    [[nodiscard]] Scene& active_scene() { return runtime_scene ? *runtime_scene : scene; }

    [[nodiscard]] const AssetRegistry& active_assets() const { return project ? project->assets : assets; }
};

}
