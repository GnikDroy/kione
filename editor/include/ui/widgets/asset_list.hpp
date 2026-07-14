#pragma once

#include <imgui.h>

#include "ui/widgets/widget.hpp"

namespace k2::editor {

class AssetListWidget : public IWidget {
    ImGuiTextFilter filter;

public:
    void render(EditorLayer&) override;
};
}
