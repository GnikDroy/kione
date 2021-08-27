#include "core/imgui_layer.hpp"
#include "rendering/renderer2D.hpp"

#include "widgets/asset_widget.hpp"
#include "widgets/component_inspector.hpp"
#include "widgets/debug_widget.hpp"
#include "widgets/entity_selector.hpp"
#include "widgets/file_explorer.hpp"
#include "widgets/log_viewer.hpp"
#include "widgets/viewport_widget.hpp"

#include "components.hpp"
#include "editor_resources.hpp"

namespace k2 {

k2::SpriteComponent sprite {
    .texture = "white"_fnv1a,
};

k2::TransformComponent transform {
    .scale { 300.0f, 300.0f, 1.0f },
};

class EditorLayer : public k2::ImguiLayer {
    k2::ComponentInspector<entt::entity> component_inspector;
    k2::EntitySelector<entt::entity> entity_selector;
    k2::editor::LogViewer log_viewer;
    k2::editor::DebugWidget debug_widget;
    k2::editor::FileExplorerWidget file_explorer_widget;
    k2::editor::ViewportWidget viewport_widget;
    k2::Renderer2D renderer2D;

    entt::registry registry;

public:
    explicit EditorLayer(k2::Window& window)
        : k2::ImguiLayer(window) {
        k2::Resources::get<k2::Texture2D>()["white"_fnv1a] = k2::Texture2D::create_white_texture();

        renderer2D.set_frame_buffer({ { .width = window.get_width(),
            .height = window.get_height(),
            .attachments {
                {
                    .buffer_type = k2::FrameBuffer::Attachment::BufferType::Texture,
                    .type = k2::FrameBuffer::Attachment::Type::Color,
                },
                {
                    .buffer_type = k2::FrameBuffer::Attachment::BufferType::Texture,
                    .type = k2::FrameBuffer::Attachment::Type::DepthStencil,
                },
            } } });

        renderer2D.camera = k2::Camera {
            .position { 0, 0, 1000.f },
            .target { 0, 0, 0 },
            .up { 0, 1.0f, 0 },

            .projection_traits { k2::Camera::OrthographicTraits {
                .left = -float(window.get_width()),
                .right = float(window.get_width()),
                .top = float(window.get_height()),
                .bottom = -float(window.get_height()),
                .far_clip = -1000.f,
                .near_clip = 1000.f,
            } },
        };

        component_inspector.register_component<k2::TransformComponent>("Transform");
        component_inspector.register_component<k2::Camera>("Camera");
        component_inspector.register_component<k2::SpriteComponent>("Sprite");
        component_inspector.register_component<k2::TagComponent>("Tag");

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
            ImGui::Begin(ICON_FA_FILE "  File Explorer", &show_file_explorer);
            file_explorer_widget.render();
            ImGui::End();
        }

        glClearColor(1.0, 1.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer2D.draw(transform, sprite);
        renderer2D.render();

        static bool show_viewport = true;
        if (show_viewport) {
            ImGui::Begin("Viewport", &show_viewport);
            viewport_widget.render(renderer2D);
            ImGui::End();
        }
    }
};

}