#pragma once

#include "ui/widgets/widget.hpp"
#include <imgui.h>
#include <nfd.hpp>

namespace k2::editor {
class MainMenuWidget : public IWidget {
    bool dark_theme = true;

public:
    void render(EditorLayer&) override;
};

}
