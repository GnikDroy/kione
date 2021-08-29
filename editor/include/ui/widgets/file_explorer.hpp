#pragma once
#include "editor_resources.hpp"
#include "ui/widgets/widget.hpp"
#include <IconsFontAwesome5.h>
#include <filesystem>

namespace k2::editor {
class FileExplorerWidget : public IWidget {
    std::filesystem::path current_directory = std::filesystem::current_path();

public:
    void render(EditorLayer&) override;

private:
    std::uint64_t predict_icon_type(const std::filesystem::directory_entry&);
    ResourceID predict_icon_texture(const std::filesystem::directory_entry&);
};

}