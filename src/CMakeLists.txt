find_package(OpenGL REQUIRED)

set(K2_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders)
set(K2_EMBEDDED_SHADERS ${CMAKE_CURRENT_BINARY_DIR}/generated/embedded_shaders.cpp)
file(GLOB K2_SHADER_FILES CONFIGURE_DEPENDS ${K2_SHADER_DIR}/*.glsl)
add_custom_command(
    OUTPUT ${K2_EMBEDDED_SHADERS}
    COMMAND ${CMAKE_COMMAND} -DSHADER_DIR=${K2_SHADER_DIR} -DOUTPUT=${K2_EMBEDDED_SHADERS}
            -P ${PROJECT_SOURCE_DIR}/cmake/embed_shaders.cmake
    DEPENDS ${K2_SHADER_FILES} ${PROJECT_SOURCE_DIR}/cmake/embed_shaders.cmake
    COMMENT "Embedding engine shaders"
)

set(K2_FONT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core/fonts)
set(K2_EMBEDDED_FONTS ${CMAKE_CURRENT_BINARY_DIR}/generated/embedded_fonts.cpp)
file(GLOB K2_FONT_FILES CONFIGURE_DEPENDS ${K2_FONT_DIR}/*.ttf)
add_custom_command(
    OUTPUT ${K2_EMBEDDED_FONTS}
    COMMAND ${CMAKE_COMMAND} -DFONT_DIR=${K2_FONT_DIR} -DOUTPUT=${K2_EMBEDDED_FONTS}
            -P ${PROJECT_SOURCE_DIR}/cmake/embed_fonts.cmake
    DEPENDS ${K2_FONT_FILES} ${PROJECT_SOURCE_DIR}/cmake/embed_fonts.cmake
    COMMENT "Embedding editor UI fonts"
)

add_library(
    kione2D
    asset/asset.cpp
    asset/asset_registry.cpp
    asset/loader.cpp
    asset/scheme.cpp
    core/animation_system.cpp
    core/app.cpp
    core/audio_clip.cpp
    core/audio_system.cpp
    core/miniaudio.cpp
    core/collision.cpp
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
    platform/desktop/paths.cpp
    rendering/batcher2D.cpp
    rendering/debug.cpp
    rendering/font.cpp
    rendering/frame_buffer.cpp
    rendering/image.cpp
    rendering/mesh.cpp
    rendering/model.cpp
    rendering/renderer2D.cpp
    rendering/shader.cpp
    rendering/shapes.cpp
    rendering/texture.cpp
    stb_image.cpp
    ${K2_EMBEDDED_SHADERS}
    ${K2_EMBEDDED_FONTS}
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
