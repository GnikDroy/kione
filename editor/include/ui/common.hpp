#pragma once
#include <algorithm>

#include "asset/asset_registry.hpp"
#include "core/resource_container.hpp"
#include "core/utils.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <imgui.h>
#include <imgui_internal.h>

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

template <class T>
void Vec3InputWidget(
    const std::string& label, glm::vec<3, T, glm::defaultp>& vec, const glm::vec<3, T, glm::defaultp>& reset = {}) {
    constexpr auto num_columns = 2;
    if (ImGui::BeginTable(label.c_str(), num_columns)) {
        ImGui::TableSetupColumn("###Items", ImGuiTableColumnFlags_WidthStretch, 3);
        ImGui::TableSetupColumn("###Labels", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableNextColumn();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        if (ImGui::Button("X")) {
            vec.x = reset.x;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##X", &vec.x, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.6f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        if (ImGui::Button("Y")) {
            vec.y = reset.y;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##Y", &vec.y, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.1f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.2f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.1f, 0.8f, 1.0f });
        if (ImGui::Button("Z")) {
            vec.z = reset.z;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGuiDrag<T>("##Z", &vec.z, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label.c_str());
        ImGui::EndTable();
    }
}

template <class T>
void Vec2InputWidget(const std::string& label, glm::vec<2, T, glm::defaultp>& vec,
    const glm::vec<2, T, glm::defaultp>& reset = { .0f }) {
    constexpr auto num_columns = 2;
    if (ImGui::BeginTable(label.c_str(), num_columns)) {
        ImGui::TableSetupColumn("###Items", ImGuiTableColumnFlags_WidthStretch, 3);
        ImGui::TableSetupColumn("###Labels", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableNextColumn();
        ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        if (ImGui::Button("X")) {
            vec.x = reset.x;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##X", &vec.x, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.6f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        if (ImGui::Button("Y")) {
            vec.y = reset.y;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##Y", &vec.y, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label.c_str());
        ImGui::EndTable();
    }
}

template <class T>
void Vec4InputWidget(const std::string& label, glm::vec<4, T, glm::defaultp>& vec,
    const glm::vec<4, T, glm::defaultp>& reset = { .0f }) {
    constexpr auto num_columns = 2;
    if (ImGui::BeginTable(label.c_str(), num_columns)) {
        ImGui::TableSetupColumn("###Items", ImGuiTableColumnFlags_WidthStretch, 3);
        ImGui::TableSetupColumn("###Labels", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableNextColumn();
        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        if (ImGui::Button("X")) {
            vec.x = reset.x;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##X", &vec.x, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.6f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        if (ImGui::Button("Y")) {
            vec.y = reset.y;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##Y", &vec.y, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.1f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.2f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.1f, 0.8f, 1.0f });
        if (ImGui::Button("Z")) {
            vec.z = reset.z;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGuiDrag<T>("##Z", &vec.z, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.5f, 0.3f, 0.6f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.6f, 0.4f, 0.7f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.5f, 0.3f, 0.6f, 1.0f });
        if (ImGui::Button("W")) {
            vec.w = reset.w;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGuiDrag<T>("##W", &vec.w, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label.c_str());
        ImGui::EndTable();
    }
}
template <arithmetic T>
void RectInputWidget(const std::string& label, k2::Rect<T>& rect, const k2::Rect<T>& reset = {}) {
    constexpr auto num_columns = 2;
    if (ImGui::BeginTable(label.c_str(), num_columns)) {
        ImGui::TableSetupColumn("###Items", ImGuiTableColumnFlags_WidthStretch, 3);
        ImGui::TableSetupColumn("###Labels", ImGuiTableColumnFlags_WidthStretch, 1);
        ImGui::TableNextColumn();
        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.10f, 1.0f });
        if (ImGui::Button("X")) {
            rect.x = reset.x;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##X", &rect.x, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.6f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.5f, 0.10f, 1.0f });
        if (ImGui::Button("Y")) {
            rect.y = reset.y;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGuiDrag<T>("##Y", &rect.y, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.1f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.2f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.1f, 0.8f, 1.0f });
        if (ImGui::Button("W")) {
            rect.w = reset.w;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGuiDrag<T>("##W", &rect.w, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.5f, 0.3f, 0.6f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.6f, 0.4f, 0.7f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.5f, 0.3f, 0.6f, 1.0f });
        if (ImGui::Button("H")) {
            rect.h = reset.h;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        ImGuiDrag<T>("##H", &rect.h, 0.1f);
        ImGui::SameLine();
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label.c_str());
        ImGui::EndTable();
    }
}

inline void OrientationInputWidget(const std::string& label, glm::quat& quaternion) {
    auto euler_angles = glm::degrees(glm::eulerAngles(quaternion));
    Vec3InputWidget(label, euler_angles);
    euler_angles = glm::radians(euler_angles);
    quaternion = glm::quat(euler_angles);
}

inline void ResourceInputWidget(const std::string& label, ResourceID& id, const AssetRegistry& asset_registry) {
    // TODO: Switch to the cpp imgui implementation of InputText
    // 64 is arbitrary here.
    // Since AssetRegistry stores strings, this can be arbitrarily long
    std::array<char, 64> input {};

    // Only recompute the ID when the user actually edits the text; otherwise the
    // placeholder or a truncated name would silently overwrite a valid ID.
    bool edited = false;
    if (asset_registry.count(id)) {
        auto& name = asset_registry.at(id).first;
        auto length = std::min(name.size(), input.size() - 1);
        std::memcpy(input.data(), name.c_str(), length);
        input[length] = 0;
        edited = ImGui::InputText(label.c_str(), input.data(), input.size());
    } else {
        std::string_view invalid_txt { "Invalid ID!" };
        auto length = std::min(invalid_txt.size(), input.size() - 1);
        std::memcpy(input.data(), invalid_txt.data(), length);
        input[length] = 0;
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        edited = ImGui::InputText(label.c_str(), input.data(), input.size());
        ImGui::PopStyleColor();
    }
    if (edited) {
        id = fnv1a(input.data());
    }
}
}
