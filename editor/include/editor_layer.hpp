#include "core/imgui_layer.hpp"

#include "ui/widgets/main_menu.hpp"
#include "ui/windows/asset.hpp"
#include "ui/windows/component_inspector.hpp"
#include "ui/windows/debug.hpp"
#include "ui/windows/entity_selector.hpp"
#include "ui/windows/file_explorer.hpp"
#include "ui/windows/log_viewer.hpp"
#include "ui/windows/viewport2D.hpp"

namespace k2 {
class EditorLayer : public k2::ImguiLayer {
public:
    k2::editor::MainMenuWidget main_menu_widget;
    k2::editor::ComponentInspectorWindow<entt::entity> component_inspector { ICON_FA_WRENCH "  Inspector" };
    k2::editor::EntitySelectorWindow<entt::entity> entity_selector { ICON_FA_BARS "  Entity Selector" };
    k2::editor::LogViewerWindow log_viewer { ICON_FA_BOOK "  Log Viewer" };
    k2::editor::DebugWindow debug_widget { ICON_FA_BUG "  Debug" };
    k2::editor::FileExplorerWindow file_explorer { ICON_FA_FILE "  File Explorer" };
    k2::editor::Viewport2DWindow viewport2D { ICON_FA_BINOCULARS "  Viewport 2D" };

    Scene scene;
    AssetRegistry assets = AssetRegistryLoader::load({ .url = "file:///res/editor.yaml" });

    explicit EditorLayer(k2::Window& window);
    void update(float) override;
};

}
