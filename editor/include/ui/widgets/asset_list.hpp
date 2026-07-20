#pragma once

#include <string>

#include <imgui.h>

#include "ui/widgets/widget.hpp"

namespace k2::editor {

class AssetListWidget : public IWidget {
    ImGuiTextFilter filter;
    std::string rename_target;
    std::string rename_buffer;
    std::string remove_target;

public:
    void render(EditorLayer&) override;
};
}
