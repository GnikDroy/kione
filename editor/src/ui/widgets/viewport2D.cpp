#include "ui/widgets/viewport2D.hpp"
#include "components/transform.hpp"
#include "editor_layer.hpp"

#include <ImGuizmo.h>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace k2::editor {
Viewport2DWidget::Viewport2DWidget()
    : gizmo_operation { ImGuizmo::TRANSLATE } {
    width = 100;
    height = 100;
    resize_frame_buffer();
}

void Viewport2DWidget::resize_frame_buffer() {
    renderer2D.set_frame_buffer({ { .width = std::size_t(width),
        .height = std::size_t(height),
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

void Viewport2DWidget::update_camera() {
    renderer2D.camera = k2::Camera {
        .position { camera_position.x, camera_position.y, 1000.f },
        .target { camera_position.x, camera_position.y, 0 },
        .up { 0, 1.0f, 0 },

        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -width * zoom,
            .right = width * zoom,
            .top = height * zoom,
            .bottom = -height * zoom,
            .far_clip = -1000.f,
            .near_clip = 1000.f,
        } },
    };
}

glm::vec2 Viewport2DWidget::screen_to_world(ImVec2 screen, ImVec2 rect_min) const {
    auto u = (screen.x - rect_min.x) / width;
    auto v = (screen.y - rect_min.y) / height;
    return { camera_position.x + (u - 0.5f) * 2.0f * width * zoom,
        camera_position.y + (0.5f - v) * 2.0f * height * zoom };
}

void Viewport2DWidget::draw_gizmo(EditorLayer& editor_layer, ImVec2 rect_min) {
    // Space is the pan modifier; the gizmo must not grab the drag.
    if (ImGui::IsKeyDown(ImGuiKey_Space)) {
        return;
    }

    auto& registry = editor_layer.scene.registry;
    auto active = editor_layer.entity_selector.get_widget().get_active();
    if (!registry.valid(active)) {
        return;
    }
    auto* transform = registry.try_get<k2::TransformComponent>(active);
    if (transform == nullptr) {
        return;
    }

    if (ImGui::IsWindowFocused()) {
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
    auto model = transform->get_matrix();

    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
            static_cast<ImGuizmo::OPERATION>(gizmo_operation), ImGuizmo::LOCAL, glm::value_ptr(model))) {
        float translation[3], rotation[3], scale[3];
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
        transform->translation = { translation[0], translation[1], translation[2] };
        transform->orientation = glm::quat(glm::radians(glm::vec3 { rotation[0], rotation[1], rotation[2] }));
        transform->scale = { scale[0], scale[1], scale[2] };
    }
}

void Viewport2DWidget::handle_interaction(EditorLayer& editor_layer, ImVec2 rect_min) {
    auto& io = ImGui::GetIO();
    bool hovered = ImGui::IsItemHovered();

    bool panning = hovered
        && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)
            || (ImGui::IsKeyDown(ImGuiKey_Space) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)));
    if (panning) {
        camera_position.x -= io.MouseDelta.x * 2.0f * zoom;
        camera_position.y += io.MouseDelta.y * 2.0f * zoom;
    }

    if (hovered && io.MouseWheel != 0.0f) {
        auto world_before = screen_to_world(io.MousePos, rect_min);
        zoom = std::clamp(zoom * std::pow(0.9f, io.MouseWheel), 0.01f, 100.0f);
        auto world_after = screen_to_world(io.MousePos, rect_min);
        camera_position += world_before - world_after;
    }

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_Space)
        && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
        auto world = screen_to_world(io.MousePos, rect_min);
        auto picked = entt::entity { entt::null };
        auto picked_z = -std::numeric_limits<float>::infinity();

        // Sprite quads are +-1 in local space; rotation is ignored for picking.
        editor_layer.scene.registry.view<k2::TransformComponent, k2::SpriteComponent>().each(
            [&](auto entity, const auto& transform, const auto&) {
                auto half = glm::abs(glm::vec2 { transform.scale });
                if (std::abs(world.x - transform.translation.x) <= half.x
                    && std::abs(world.y - transform.translation.y) <= half.y
                    && transform.translation.z >= picked_z) {
                    picked = entity;
                    picked_z = transform.translation.z;
                }
            });

        editor_layer.entity_selector.get_widget().set_active(picked);
    }
}

void Viewport2DWidget::render(EditorLayer& editor_layer) {
    auto& scene = editor_layer.scene;

    auto space = ImGui::GetContentRegionAvail();
    // Framebuffers cannot be 0x0
    space.x = std::max(1.f, space.x);
    space.y = std::max(1.f, space.y);

    if (space.x != width || space.y != height) {
        width = space.x;
        height = space.y;
        resize_frame_buffer();
    }

    update_camera();

    renderer2D.set_clear_color(0.2f, 0.2f, 0.2f, 1.0f);
    renderer2D.clear();
    renderer2D.draw(scene);
    renderer2D.render();

    // Show the color attachment in viewport
    std::uint32_t texture_id = renderer2D.get_frame_buffer().get_traits().attachments.front().id;

    ImGui::Image((std::uint64_t)texture_id,
        { (float)renderer2D.get_frame_buffer().get_traits().width,
            (float)renderer2D.get_frame_buffer().get_traits().height },
        { 0, 1 }, { 1, 0 });

    auto rect_min = ImGui::GetItemRectMin();
    draw_gizmo(editor_layer, rect_min);
    handle_interaction(editor_layer, rect_min);
}
}
