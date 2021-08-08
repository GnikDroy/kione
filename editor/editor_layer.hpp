#include "core/imgui_layer.hpp"

#include "widgets/asset_widget.hpp"
#include "widgets/component_inspector.hpp"
#include "widgets/debug_widget.hpp"
#include "widgets/entity_selector.hpp"
#include "widgets/file_explorer.hpp"
#include "widgets/log_viewer.hpp"

#include "components.hpp"
#include "editor_resources.hpp"

namespace k2 {
class EditorLayer : public k2::ImguiLayer {
    k2::ComponentInspector<entt::entity> component_inspector;
    k2::EntitySelector<entt::entity> entity_selector;
    k2::editor::LogViewer log_viewer;
    k2::editor::DebugWidget debug_widget;
    k2::editor::FileExplorerWidget file_explorer_widget;

    entt::registry registry;

public:
    explicit EditorLayer(k2::Window& window)
        : k2::ImguiLayer(window) {
        component_inspector.register_component<Transform>("Transform");
        component_inspector.register_component<DirectionalLight>("Directional Light");
        component_inspector.register_component<PointLight>("Point Light");

        for (size_t i = 0; i < 10; i++) {
            auto entity = registry.create();
            registry.emplace<Transform>(entity);
            registry.emplace<DirectionalLight>(entity);
        }
        auto var = AssetRegistry({ .url = "file:///res/icons/bundle.yaml" });
        k2::Log::core().info(fmt::format("Size: {}", var.assets.size()));
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
        static bool show_debug = true;
        if (show_debug) {
            ImGui::Begin(ICON_FA_BUG "  Debug", &show_debug);
            debug_widget.render();
            ImGui::End();
        }

        static bool show_entity_selector = true;
        if (show_entity_selector) {
            ImGui::Begin(ICON_FA_BARS "  Entity Selector", &show_entity_selector);
            entity_selector.render(registry);
            ImGui::End();
        }

        static bool show_log = true;
        if (show_log) {
            ImGui::Begin(ICON_FA_BOOK "  Log Viewer", &show_log);
            log_viewer.render(theme.get());
            ImGui::End();
        }

        static bool show_component_inspector = true;
        if (show_component_inspector) {
            ImGui::Begin(ICON_FA_WRENCH "  Inspector", &show_component_inspector);
            component_inspector.render(registry, entity_selector.get_active());
            ImGui::End();
        }

        static bool show_file_explorer = true;
        if (show_file_explorer) {
            ImGui::Begin(ICON_FA_FILE "  File Explorer");
            file_explorer_widget.render();
            ImGui::End();
        }
    }
};

}