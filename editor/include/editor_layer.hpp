#include "core/imgui_layer.hpp"

#include "ui/widgets/main_menu.hpp"
#include "ui/windows/asset.hpp"
#include "ui/windows/component_inspector.hpp"
#include "ui/windows/debug.hpp"
#include "ui/windows/entity_selector.hpp"
#include "ui/windows/file_explorer.hpp"
#include "ui/windows/log_viewer.hpp"
#include "ui/windows/viewport2D.hpp"

#include "editor_resources.hpp"

namespace k2 {
class EditorLayer : public k2::ImguiLayer {
public:
    k2::editor::MainMenuWidget main_menu_widget;

    k2::editor::ComponentInspectorWindow<entt::entity> component_inspector { "Inspector" };
    k2::editor::EntitySelectorWindow<entt::entity> entity_selector { "Entity Selector" };
    k2::editor::LogViewerWindow log_viewer { "Log Viewer" };
    k2::editor::DebugWindow debug_widget { "Debug Widget" };
    k2::editor::FileExplorerWindow file_explorer { "File Explorer" };
    k2::editor::Viewport2DWindow viewport2D { "Viewport 2D" };

    Scene scene;

    explicit EditorLayer(k2::Window& window)
        : k2::ImguiLayer(window) {
        using namespace k2::literals;
        k2::Resources::get<k2::Texture2D>()["white"_fnv1a] = k2::Texture2D::create_white_texture();

        auto var = AssetRegistry({ .url = "file:///res/icons/bundle.yaml" });
        for (auto& [id, asset] : var.assets) {
            k2::Log::core().trace(
                fmt::format("Name: '{}', URL: '{}', Type: '{}'", id, asset.url, asset.get_type_strv()));
        }

        for (auto& [id, asset] : var.assets) {
            if (asset.type == Asset::Type::Image) {
                auto image = AssetLoader<Asset::Type::Image>::get_resource(asset);
                editor::Resources::get<Texture2D>()[fnv1a(id)] = Texture2D { image };
                editor::Resources::get<Image>()[fnv1a(id)] = std::move(image);
            }
        }
    }

    void update(float) override {
        ImGui::DockSpaceOverViewport();
        main_menu_widget.render(*this);
        component_inspector.render(*this);
        entity_selector.render(*this);
        log_viewer.render(*this);
        debug_widget.render(*this);
        file_explorer.render(*this);
        viewport2D.render(*this);
    }
};

}