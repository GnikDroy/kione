#include "editor_layer.hpp"
#include "editor_resources.hpp"

namespace k2 {
EditorLayer::EditorLayer(k2::Window& window)
    : k2::ImguiLayer(window) {
    using namespace k2::literals;
    k2::Resources::get<k2::Texture2D>()["white"_fnv1a] = k2::Texture2D::create_white_texture();

    for (auto& [id, pair] : assets.assets) {
        auto&& [name, asset] = pair;
        k2::Log::core().trace(fmt::format("Name: '{}', URL: '{}', Type: '{}'", name, asset.url, asset.get_type_strv()));
    }

    for (auto& [id, pair] : assets.assets) {
        auto&& [name, asset] = pair;
        if (asset.type == Asset::Type::Image) {
            auto image = AssetLoader::get<k2::Image>(asset);
            editor::Resources::get<Texture2D>()[id] = Texture2D { image };
            k2::Resources::get<Texture2D>()[id] = Texture2D { image };

            editor::Resources::get<Image>()[id] = std::move(image);
        }
    }
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
