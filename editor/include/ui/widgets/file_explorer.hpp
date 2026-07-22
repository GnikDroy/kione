#pragma once
#include "core/resources.hpp"
#include "ui/widgets/widget.hpp"
#include <IconsFontAwesome5.h>
#include <filesystem>
#include <imgui.h>

namespace k2::editor {

/**
 * @brief A widget to explore the file system.
 */
class FileExplorerWidget : public IWidget {
    /**
     * @brief The current directory being explored.
     */
    std::filesystem::path current_directory = std::filesystem::current_path();

    /**
     * @brief Pair of possibly the current directory and its directory entries.
     */
    std::pair<std::filesystem::path, std::vector<std::filesystem::directory_entry>> cached_entries;

    /**
     * @brief The size of the icons.
     */
    static constexpr auto icon_size = 75.f;

    /**
     * @brief The padding between icons.
     */
    static constexpr auto icon_padding = 20.f;

    /**
     * @brief Filter for the file names.
     */
    ImGuiTextFilter filter;

    /**
     * @brief Whether dotfiles are listed.
     */
    bool show_hidden = false;

public:
    /**
     * @brief Renders the file explorer widget.
     * @param layer The editor layer.
     */
    void render(EditorLayer&) override;

    /**
     * @brief Navigates the explorer to a directory.
     * @param directory The directory to navigate to
     */
    void set_directory(std::filesystem::path directory) { current_directory = std::move(directory); }

private:
    /**
     * @brief Caches the directory entries if the current directory has changed.
     */
    void cache_entries();

    /**
     * @brief Renders the current path as clickable segments.
     */
    void render_breadcrumbs();

    /**
     * @brief Renders the directory table.
     * @param editor_layer The editor layer.
     */
    void render_directory_table(EditorLayer& editor_layer);

    /**
     * @brief Renders the directory.
     *
     * @param editor_layer The editor layer.
     * @param entry The directory entry.
     */
    void render_directory(EditorLayer& editor_layer, const std::filesystem::directory_entry&);
};

}
