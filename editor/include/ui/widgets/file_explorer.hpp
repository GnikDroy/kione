#pragma once
#include "editor_resources.hpp"
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

public:
    /**
     * @brief Renders the file explorer widget.
     * @param layer The editor layer.
     */
    void render(EditorLayer&) override;

private:
    /**
     * @brief Caches the directory entries if the current directory has changed.
     */
    void cache_entries();

    /**
     * @brief Renders the directory table.
     */
    void render_directory_table();

    /**
     * @brief Renders the directory.
     *
     * @param entry The directory entry.
     */
    void render_directory(const std::filesystem::directory_entry&);

    /**
     * @brief Predict the icon type based on the directory entry.
     *
     * @param entry The directory entry.
     * @return The fnv1a hash of the icon type.
     */
    std::uint64_t predict_icon_type(const std::filesystem::directory_entry&);

    /**
     * @brief Predict the icon texture based on the directory entry.
     *
     * @param entry The directory entry.
     * @return The resource ID of the icon texture.
     */
    ResourceID predict_icon_texture(const std::filesystem::directory_entry&);
};

}