#pragma once

#include "core/logger.hpp"
#include "ui/widgets/widget.hpp"
#include <imgui.h>
#include <nfd.hpp>

namespace k2::editor {
class MainMenuWidget : public IWidget {
public:
    void render(EditorLayer&) override;
};

}