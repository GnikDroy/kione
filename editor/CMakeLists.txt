add_executable(
    editor
    src/main.cpp
    src/editor_layer.cpp
    src/ui/widgets/animation_editor.cpp
    src/ui/widgets/asset.cpp
    src/ui/widgets/asset_list.cpp
    src/ui/widgets/component_inspector.cpp
    src/ui/widgets/entity_selector.cpp
    src/ui/widgets/file_explorer.cpp
    src/ui/widgets/log_viewer.cpp
    src/ui/widgets/main_menu.cpp
    src/ui/widgets/project_settings.cpp
    src/ui/widgets/tilemap_editor.cpp
    src/ui/widgets/tileset_editor.cpp
    src/ui/widgets/viewport2D.cpp
)
add_executable(kione2D::editor ALIAS editor)

target_include_directories(editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_precompile_headers(editor PRIVATE
    <algorithm>
    <any>
    <cassert>
    <cmath>
    <concepts>
    <cstddef>
    <cstdint>
    <filesystem>
    <fstream>
    <iostream>
    <limits>
    <map>
    <memory>
    <numeric>
    <optional>
    <queue>
    <ranges>
    <regex>
    <set>
    <span>
    <sstream>
    <stdexcept>
    <string>
    <string_view>
    <tuple>
    <type_traits>
    <unordered_map>
    <unordered_set>
    <utility>
    <variant>
    <vector>
    <format>

    [["entt/entt.hpp"]]
)

target_link_libraries(editor kione2D::kione2D ImGuizmo::ImGuizmo)

set_target_properties(editor
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 23
)

if (WIN32)
    set_target_properties(editor
        PROPERTIES
        WIN32_EXECUTABLE true
    )
endif ()

# /wd4103 due to a msvc compiler bug
# /wd5054 rapidjson deprecated warning
# see https://developercommunity.visualstudio.com/t/warning-c4103-in-visual-studio-166-update/1057589
target_compile_options(editor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest /wd4201 /wd4103 /wd5054>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:editor>/res/
    COMMENT "COPY res directory to build"
)
