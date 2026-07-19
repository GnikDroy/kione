#pragma once
#include "asset/asset_handle.hpp"
#include "asset/asset_registry.hpp"
#include "core/utils.hpp"
#include <array>
#include <cfloat>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace k2::editor {

template <k2::arithmetic T, class... Args> constexpr auto ImGuiDrag(Args&&... args) {
    if constexpr (std::is_same_v<T, int>) {
        return ImGui::DragInt(std::forward<Args>(args)...);
    } else if constexpr (std::is_same_v<T, float>) {
        return ImGui::DragFloat(std::forward<Args>(args)...);
    } else {
        static_assert(k2::always_false<T>, "Arithmetic type not implemented.");
    }
}

inline bool BeginPropertyTable(const char* id) {
    if (!ImGui::BeginTable(id, 2)) {
        return false;
    }
    ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthStretch, 2.0f);
    ImGui::TableSetupColumn("##field", ImGuiTableColumnFlags_WidthStretch, 5.0f);
    return true;
}

inline void PropertyLabel(const char* label) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN);
}

inline void EndPropertyTable() { ImGui::EndTable(); }

namespace detail {
    struct Axis {
        const char* name;
        ImVec4 color;
    };

    inline constexpr Axis axis_x { .name = "X", .color { 0.75f, 0.31f, 0.33f, 1.0f } };
    inline constexpr Axis axis_y { .name = "Y", .color { 0.36f, 0.58f, 0.32f, 1.0f } };
    inline constexpr Axis axis_z { .name = "Z", .color { 0.27f, 0.42f, 0.70f, 1.0f } };
    inline constexpr Axis axis_w { .name = "W", .color { 0.55f, 0.39f, 0.67f, 1.0f } };

    template <k2::arithmetic T, std::size_t N>
    void MultiAxisField(const char* id, const std::array<Axis, N>& axes, const std::array<T*, N>& values,
        const std::array<T, N>& resets, float speed) {
        ImGui::PushID(id);
        constexpr auto gap = 4.0f;
        auto slot = (ImGui::GetContentRegionAvail().x - gap * (N - 1)) / N;
        for (std::size_t i = 0; i < N; i++) {
            if (i != 0) {
                ImGui::SameLine(0.0f, gap);
            }
            ImGui::PushID(static_cast<int>(i));
            const auto& color = axes[i].color;
            ImVec4 hovered { std::min(color.x * 1.2f, 1.0f), std::min(color.y * 1.2f, 1.0f),
                std::min(color.z * 1.2f, 1.0f), 1.0f };
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
            if (ImGui::Button(axes[i].name)) {
                *values[i] = resets[i];
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::SetNextItemWidth(std::max(slot - ImGui::GetItemRectSize().x, 40.0f));
            ImGuiDrag<T>("##value", values[i], speed);
            ImGui::PopID();
        }
        ImGui::PopID();
    }
}

template <class T>
void Vec2Field(const char* id, glm::vec<2, T, glm::defaultp>& vec, const glm::vec<2, T, glm::defaultp>& reset = {},
    float speed = 0.1f) {
    detail::MultiAxisField<T, 2>(
        id, { detail::axis_x, detail::axis_y }, { &vec.x, &vec.y }, { reset.x, reset.y }, speed);
}

template <class T>
void Vec3Field(const char* id, glm::vec<3, T, glm::defaultp>& vec, const glm::vec<3, T, glm::defaultp>& reset = {},
    float speed = 0.1f) {
    detail::MultiAxisField<T, 3>(id, { detail::axis_x, detail::axis_y, detail::axis_z }, { &vec.x, &vec.y, &vec.z },
        { reset.x, reset.y, reset.z }, speed);
}

template <class T>
void Vec4Field(const char* id, glm::vec<4, T, glm::defaultp>& vec, const glm::vec<4, T, glm::defaultp>& reset = {},
    float speed = 0.1f) {
    detail::MultiAxisField<T, 4>(id, { detail::axis_x, detail::axis_y, detail::axis_z, detail::axis_w },
        { &vec.x, &vec.y, &vec.z, &vec.w }, { reset.x, reset.y, reset.z, reset.w }, speed);
}

template <arithmetic T>
void RectField(const char* id, k2::Rect<T>& rect, const k2::Rect<T>& reset = {}, float speed = 0.1f) {
    detail::MultiAxisField<T, 4>(id,
        { detail::axis_x, detail::axis_y, detail::Axis { .name = "W", .color = detail::axis_z.color },
            detail::Axis { .name = "H", .color = detail::axis_w.color } },
        { &rect.x, &rect.y, &rect.w, &rect.h }, { reset.x, reset.y, reset.w, reset.h }, speed);
}

inline void RotationField(const char* id, glm::quat& quaternion) {
    auto euler = glm::degrees(glm::eulerAngles(quaternion));
    Vec3Field(id, euler, {}, 0.5f);
    quaternion = glm::quat(glm::radians(euler));
}

inline void ResourceInputWidget(const std::string& label, AssetHandle& handle, const AssetRegistry& asset_registry,
    Asset::Type type, bool can_be_none = true) {
    bool dangling = handle.name.empty() ? !can_be_none : asset_registry.count(handle.id) == 0;
    if (dangling) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

    const char* preview = handle.name.empty() ? "<none>" : handle.name.c_str();
    if (ImGui::BeginCombo(label.c_str(), preview)) {
        if (can_be_none && ImGui::Selectable("<none>", handle.name.empty())) {
            handle.set("");
        }

        std::vector<const std::string*> names;
        for (auto& [id, pair] : asset_registry) {
            if (pair.second.type == type) {
                names.push_back(&pair.first);
            }
        }
        std::ranges::sort(names, {}, [](const std::string* name) -> const std::string& { return *name; });

        for (auto* name : names) {
            bool selected = handle.name == *name;
            if (ImGui::Selectable(name->c_str(), selected)) {
                handle.set(*name);
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (dangling) {
        ImGui::PopStyleColor();
    }
}
}
