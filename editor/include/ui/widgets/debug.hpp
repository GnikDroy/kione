#pragma once

#include "ui/widgets/widget.hpp"

namespace k2::editor {
class DebugWidget : public IWidget {
    bool show_demo_window = false;

public:
    void render(EditorLayer&) override;
};
}