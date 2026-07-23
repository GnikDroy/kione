#include "ui/widgets/viewport2D.hpp"
#include "components/collider.hpp"
#include "components/transform.hpp"
#include "core/collision.hpp"
#include "editor_layer.hpp"
#include "rendering/draw_list.hpp"

#include <IconsMaterialSymbols.h>
#include <ImGuizmo.h>
#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <variant>

namespace k2::editor {
Viewport2DWidget::Viewport2DWidget()
    : gizmo_operation { ImGuizmo::TRANSLATE } {
    width = 100;
    height = 100;
    resize_frame_buffer();
}

void Viewport2DWidget::resize_frame_buffer() {
    renderer2D.set_frame_buffer({ { .width = std::size_t(width * dpi_scale),
        .height = std::size_t(height * dpi_scale),
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
}

k2::Camera Viewport2DWidget::make_camera() const {
    return k2::Camera {
        .position { camera_position.x, camera_position.y, 1000.f },
        .target { camera_position.x, camera_position.y, 0 },
        .up { 0, 1.0f, 0 },

        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -width * 0.5f * zoom,
            .right = width * 0.5f * zoom,
            .top = height * 0.5f * zoom,
            .bottom = -height * 0.5f * zoom,
            .far_clip = 0.f,
            .near_clip = 2000.f,
        } },
    };
}

void Viewport2DWidget::update_camera() { renderer2D.camera = make_camera(); }

void Viewport2DWidget::draw_gizmo(EditorLayer& editor_layer, ImVec2 rect_min) {
    // Space is the pan modifier; the gizmo must not grab the drag.
    if (ImGui::IsKeyDown(ImGuiKey_Space)) {
        return;
    }

    auto& registry = editor_layer.active_scene().registry;
    auto active = editor_layer.entity_selector.get_widget().get_active();
    if (!registry.valid(active)) {
        return;
    }
    auto* transform = registry.try_get<k2::TransformComponent>(active);
    if (transform == nullptr) {
        return;
    }

    if (ImGui::IsWindowFocused() && !editor_layer.is_playing()) {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            gizmo_operation = ImGuizmo::TRANSLATE;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            gizmo_operation = ImGuizmo::ROTATE;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            gizmo_operation = ImGuizmo::SCALE;
        }
    }

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rect_min.x, rect_min.y, width, height);

    auto view = renderer2D.camera.get_view();
    auto projection = renderer2D.camera.get_projection();
    auto parent_world = k2::TransformComponent::parent_world(registry, active);
    auto model = parent_world * transform->get_matrix();

    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
            static_cast<ImGuizmo::OPERATION>(gizmo_operation), ImGuizmo::LOCAL, glm::value_ptr(model))) {
        auto z = transform->translation.z;
        transform->set_from_matrix(glm::inverse(parent_world) * model);
        transform->translation.z = z;
    }
}

void Viewport2DWidget::handle_interaction(EditorLayer& editor_layer, ImVec2 rect_min) {
    auto& io = ImGui::GetIO();
    bool hovered = ImGui::IsItemHovered();

    if (ImGui::IsWindowFocused() && !editor_layer.is_playing()) {
        if (ImGui::IsKeyPressed(ImGuiKey_F)) {
            auto& registry = editor_layer.active_scene().registry;
            auto active = editor_layer.entity_selector.get_widget().get_active();
            if (registry.valid(active) && registry.all_of<k2::TransformComponent>(active)) {
                auto world_matrix = k2::TransformComponent::world(registry, active);
                focus({ world_matrix[3][0], world_matrix[3][1] });
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
            camera_position = {};
            zoom = 1.0f;
        }
    }

    bool panning = hovered
        && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)
            || (ImGui::IsKeyDown(ImGuiKey_Space) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)));
    if (panning) {
        camera_position.x -= io.MouseDelta.x * zoom;
        camera_position.y += io.MouseDelta.y * zoom;
    }

    glm::vec2 mouse { io.MousePos.x, io.MousePos.y };
    if (hovered && io.MouseWheel != 0.0f) {
        auto world_before = k2::SceneView { .camera = make_camera(), .viewport = viewport(rect_min) }.screen_to_world(mouse);
        zoom = std::clamp(zoom * std::pow(0.9f, io.MouseWheel), 0.01f, 100.0f);
        auto world_after = k2::SceneView { .camera = make_camera(), .viewport = viewport(rect_min) }.screen_to_world(mouse);
        camera_position += world_before - world_after;
    }

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_Space)
        && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
        auto world = k2::SceneView { .camera = make_camera(), .viewport = viewport(rect_min) }.screen_to_world(mouse);
        auto picked = entt::entity { entt::null };
        auto picked_z = -std::numeric_limits<float>::infinity();

        auto& registry = editor_layer.active_scene().registry;
        registry.view<k2::TransformComponent, k2::SpriteComponent>().each([&](auto entity, const auto&, const auto& sprite) {
            auto world_matrix = k2::TransformComponent::world(registry, entity);
            auto local = glm::inverse(world_matrix) * glm::vec4 { world.x, world.y, world_matrix[3][2], 1.0f };
            if (std::abs(local.x) <= sprite.size.x * 0.5f && std::abs(local.y) <= sprite.size.y * 0.5f
                && world_matrix[3][2] >= picked_z) {
                picked = entity;
                picked_z = world_matrix[3][2];
            }
        });

        editor_layer.entity_selector.get_widget().set_active(picked);
    }
}

bool Viewport2DWidget::draw_toolbar(EditorLayer& editor_layer, ImVec2 rect_min) {
    ImGui::SetNextWindowPos({ rect_min.x + width * 0.5f, rect_min.y + 8.0f }, ImGuiCond_Always, { 0.5f, 0.0f });
    ImGui::SetNextWindowBgAlpha(0.35f);
    constexpr auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoFocusOnAppearing;

    bool started = false;
    if (ImGui::Begin("##viewport_toolbar", nullptr, flags)) {
        bool playing = editor_layer.is_playing();
        auto base = playing ? ImVec4 { 0.75f, 0.15f, 0.15f, 1.0f } : ImVec4 { 0.15f, 0.60f, 0.20f, 1.0f };
        auto hovered = playing ? ImVec4 { 0.85f, 0.25f, 0.25f, 1.0f } : ImVec4 { 0.20f, 0.70f, 0.25f, 1.0f };

        ImGui::PushStyleColor(ImGuiCol_Button, base);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, base);

        if (ImGui::Button(playing ? ICON_MS_STOP : ICON_MS_PLAY_ARROW, { 36.0f, 0 })) {
            playing ? editor_layer.stop() : editor_layer.play();
            started = !playing;
        }

        ImGui::PopStyleColor(3);
    }
    ImGui::End();
    return started;
}

void Viewport2DWidget::push_collider_overlay(EditorLayer& editor_layer) {
    auto& registry = editor_layer.active_scene().registry;
    auto view = registry.view<k2::ColliderComponent, k2::TransformComponent>();
    if (view.begin() == view.end()) {
        return;
    }

    auto rotate = [](glm::vec2 vec, float angle) {
        float cos_a = std::cos(angle);
        float sin_a = std::sin(angle);
        return glm::vec2 { cos_a * vec.x - sin_a * vec.y, sin_a * vec.x + cos_a * vec.y };
    };

    auto& draw_list = registry.ctx().emplace<k2::DrawList>();
    auto active = editor_layer.entity_selector.get_widget().get_active();
    constexpr float overlay_z = 950.0f;
    constexpr float thickness = 1.5f;

    for (auto [entity, collider, transform] : view.each()) {
        glm::vec4 color { 0.3f, 1.0f, 0.4f, entity == active ? 0.9f : 0.35f };
        auto world = k2::collision::world_collider(collider, transform);

        if (const auto* circle = std::get_if<k2::CircleShape>(&collider.shape)) {
            draw_list.commands.push_back({ .kind = k2::DrawCommand::Kind::Circle,
                .a = world.center,
                .radius = circle->radius,
                .width = thickness,
                .color = color,
                .z = overlay_z,
                .filled = false });
        } else if (const auto* box = std::get_if<k2::BoxShape>(&collider.shape)) {
            auto half = box->size * 0.5f;
            draw_list.commands.push_back({ .kind = k2::DrawCommand::Kind::Polygon,
                .width = thickness,
                .color = color,
                .z = overlay_z,
                .filled = false,
                .closed = true,
                .points = { world.center + rotate({ -half.x, -half.y }, world.angle),
                    world.center + rotate({ half.x, -half.y }, world.angle),
                    world.center + rotate({ half.x, half.y }, world.angle),
                    world.center + rotate({ -half.x, half.y }, world.angle) } });
        } else if (const auto* pill = std::get_if<k2::PillShape>(&collider.shape)) {
            auto up = rotate({ 0.0f, pill->half_height }, world.angle);
            auto side = rotate({ pill->radius, 0.0f }, world.angle);
            for (auto cap : { world.center + up, world.center - up }) {
                draw_list.commands.push_back({ .kind = k2::DrawCommand::Kind::Circle,
                    .a = cap,
                    .radius = pill->radius,
                    .width = thickness,
                    .color = color,
                    .z = overlay_z,
                    .filled = false });
            }
            for (auto sign : { 1.0f, -1.0f }) {
                draw_list.commands.push_back({ .kind = k2::DrawCommand::Kind::Line,
                    .a = world.center + up + side * sign,
                    .b = world.center - up + side * sign,
                    .width = thickness,
                    .color = color,
                    .z = overlay_z });
            }
        }
    }
}

void Viewport2DWidget::push_camera_overlay(EditorLayer& editor_layer) {
    auto& registry = editor_layer.active_scene().registry;
    auto view = registry.view<k2::Camera, k2::MainCamera>();
    if (view.begin() == view.end()) {
        return;
    }

    auto& draw_list = registry.ctx().emplace<k2::DrawList>();
    for (auto entity : view) {
        auto inverse_vp = glm::inverse(view.get<k2::Camera>(entity).get_view_projection());
        std::vector<glm::vec2> corners;
        for (auto ndc : { glm::vec2 { -1, -1 }, glm::vec2 { 1, -1 }, glm::vec2 { 1, 1 }, glm::vec2 { -1, 1 } }) {
            auto world = inverse_vp * glm::vec4 { ndc, 0.0f, 1.0f };
            corners.push_back(glm::vec2 { world } / world.w);
        }
        draw_list.commands.push_back({ .kind = k2::DrawCommand::Kind::Polygon,
            .width = 2.0f,
            .color = { 1.0f, 0.75f, 0.25f, 0.9f },
            .z = 949.0f,
            .filled = false,
            .closed = true,
            .points = std::move(corners) });
    }
}

void Viewport2DWidget::render(EditorLayer& editor_layer) {
    auto& scene = editor_layer.active_scene();

    auto space = ImGui::GetContentRegionAvail();
    space.x = std::max(1.f, space.x);
    space.y = std::max(1.f, space.y);
    auto scale = ImGui::GetIO().DisplayFramebufferScale;
    float fb_scale = std::max(1.0f, std::min(scale.x, scale.y));

    if (space.x != width || space.y != height || fb_scale != dpi_scale) {
        width = space.x;
        height = space.y;
        dpi_scale = fb_scale;
        resize_frame_buffer();
    }

    update_camera();

    k2::Rect<float> content { .x = 0.0f, .y = 0.0f, .w = width, .h = height };
    if (editor_layer.is_playing()) {
        if (const auto* main_camera = k2::find_main_camera(scene.registry)) {
            auto resolved = main_camera->for_surface(width, height);
            renderer2D.camera = resolved.camera;
            content = resolved.viewport;
        }
    }

    if (!editor_layer.is_playing()) {
        push_collider_overlay(editor_layer);
        push_camera_overlay(editor_layer);
    }

    auto backdrop = editor_layer.theme->color("surface");
    renderer2D.set_clear_color(backdrop.x, backdrop.y, backdrop.z, backdrop.w);
    renderer2D.clear();
    renderer2D.draw(scene);
    renderer2D.render();

    std::uint32_t texture_id = renderer2D.get_frame_buffer().get_traits().attachments.front().id;

    ImGui::Image((std::uint64_t)texture_id, { width, height }, { 0, 1 }, { 1, 0 });

    auto rect_min = ImGui::GetItemRectMin();
    draw_gizmo(editor_layer, rect_min);
    handle_interaction(editor_layer, rect_min);
    // Clicking Play focuses the toolbar overlay; hand focus straight to the
    // viewport so game input flows without an extra click.
    if (draw_toolbar(editor_layer, rect_min)) {
        ImGui::SetWindowFocus();
    }

    if (editor_layer.is_playing()) {
        editor_layer.runtime.scripts.set_input_enabled(ImGui::IsWindowFocused());
        scene.registry.ctx().insert_or_assign(k2::SceneView {
            .camera = renderer2D.camera,
            .viewport = { .x = rect_min.x + content.x, .y = rect_min.y + content.y, .w = content.w, .h = content.h },
        });
    }
}
}
