find_package(OpenGL REQUIRED)

add_library(
    kione2D
    asset/asset.cpp
    asset/asset_registry.cpp
    asset/loader.cpp
    asset/scheme.cpp
    core/animation_system.cpp
    core/app.cpp
    core/audio.cpp
    core/entity_ops.cpp
    core/imgui_layer.cpp
    core/imgui_theme.cpp
    core/layer.cpp
    core/logger.cpp
    core/project.cpp
    core/scene_loader.cpp
    core/script/bindings.cpp
    core/script/event_translation.cpp
    core/script/key_names.cpp
    core/script/lua_component.cpp
    core/script_system.cpp
    events/event.cpp
    platform/desktop/window.cpp
    platform/desktop/window_impl.cpp
    platform/desktop/keyboard.cpp
    platform/desktop/mouse.cpp
    platform/desktop/clipboard.cpp
    rendering/debug.cpp
    rendering/font.cpp
    rendering/frame_buffer.cpp
    rendering/image.cpp
    rendering/mesh.cpp
    rendering/model.cpp
    rendering/renderer2D.cpp
    rendering/shader.cpp
    rendering/texture.cpp
    stb_image.cpp
)
add_library(kione2D::kione2D ALIAS kione2D)

target_include_directories(kione2D
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)
target_compile_definitions(kione2D PUBLIC SOL_ALL_SAFETIES_ON=1)
target_precompile_headers(kione2D PRIVATE
    <iostream>
    <memory>
    <vector>
    <string>
    <unordered_map>
    <map>
    <set>
    <unordered_set>
    <cstdint>
    <cmath>
    <numeric>
    <algorithm>
    <cassert>
    <stdexcept>
    <type_traits>
    <concepts>

    [["entt/entt.hpp"]]

    [["spdlog/spdlog.h"]]
    [["spdlog/sinks/stdout_color_sinks.h"]]
    [["spdlog/sinks/basic_file_sink.h"]]
)

target_link_libraries(kione2D
    OpenGL::GL
    glfw glad
    glm::glm
    ImGui::ImGui
    IconFontCppHeaders::IconFontCppHeaders
    nfd

    stb::stb
    miniaudio::miniaudio
    tinyobjloader
    lua::lua
    sol2::sol2

    EnTT::EnTT
    yaml-cpp

    spdlog::spdlog
)

set_target_properties(kione2D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 23
)

target_compile_options(kione2D PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest /wd4201>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)
