#include "asset/asset_registry.hpp"
#include "core/imgui_layer.hpp"
#include "core/project.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"
#include "core/script_system.hpp"

#include <filesystem>
#include <optional>

#include "ui/widgets/main_menu.hpp"
#include "ui/windows/asset_list.hpp"
#include "ui/windows/component_inspector.hpp"
#include "ui/windows/debug.hpp"
#include "ui/windows/entity_selector.hpp"
#include "ui/windows/file_explorer.hpp"
#include "ui/windows/log_viewer.hpp"
#include "ui/windows/project_settings.hpp"
#include "ui/windows/viewport2D.hpp"

namespace k2 {
class EditorLayer : public k2::ImguiLayer {
public:
    Scene scene;
    std::optional<Scene> runtime_scene;
    AssetRegistry assets = AssetRegistryLoader::load({ .url = "file:///res/editor.yaml", .type = Asset::Type::AssetBundle });
    std::optional<Project> project;
    ResourceManager resources;
    ScriptSystem scripts;

    k2::editor::MainMenuWidget main_menu_widget;
    k2::editor::ComponentInspectorWindow<entt::entity> component_inspector { ICON_FA_WRENCH "  Inspector" };
    k2::editor::EntitySelectorWindow<entt::entity> entity_selector { ICON_FA_BARS "  Entity Selector" };
    k2::editor::LogViewerWindow log_viewer { ICON_FA_BOOK "  Log Viewer" };
    k2::editor::DebugWindow debug_widget { ICON_FA_BUG "  Debug" };
    k2::editor::FileExplorerWindow file_explorer { ICON_FA_FILE "  File Explorer" };
    k2::editor::Viewport2DWindow viewport2D { ICON_FA_BINOCULARS "  Viewport 2D" };
    k2::editor::ProjectSettingsWindow project_settings { ICON_FA_COG "  Project Settings" };
    k2::editor::AssetListWindow asset_list { ICON_FA_ARCHIVE "  Assets" };

    explicit EditorLayer(k2::Window& window);
    void begin_frame() override;
    void update(float) override;

private:
    void build_default_layout(unsigned int dockspace_id);

    void load_image_resources(const AssetRegistry& asset_registry);

    bool layout_pending = !std::filesystem::exists("imgui.ini");

public:

    void open_project(const std::filesystem::path& path);

    void create_project(const std::filesystem::path& path);

    void reload_assets();

    void request_exit();

    void reset_layout() { layout_pending = true; }

    void play();
    void stop();
    [[nodiscard]] bool is_playing() const { return runtime_scene.has_value(); }
    [[nodiscard]] Scene& active_scene() { return runtime_scene ? *runtime_scene : scene; }

    // The opened project's assets; the editor's own UI assets otherwise.
    [[nodiscard]] const AssetRegistry& active_assets() const { return project ? project->assets : assets; }
};

}
