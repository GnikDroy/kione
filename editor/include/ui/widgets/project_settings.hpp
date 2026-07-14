#pragma once

#include <filesystem>
#include <string>

#include "ui/widgets/widget.hpp"

namespace k2::editor {

class ProjectSettingsWidget : public IWidget {
    std::filesystem::path loaded_file;
    std::string name, main_scene;

public:
    void render(EditorLayer&) override;
};
}
