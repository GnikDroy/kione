add_executable(
    K2_tests

    core/animation.cpp
    core/audio.cpp
    core/project.cpp
    core/resource_container.cpp
    core/resource_manager.cpp
    core/scene_loader.cpp
    core/transforms.cpp
    core/utils.cpp
    core/imgui_theme.cpp
    core/fnv.cpp
    core/key_names.cpp
    core/script_constants.cpp
    core/script_globals.cpp
    core/collision.cpp
    core/entity_ops.cpp
    core/lua_entity.cpp
    core/lua_component.cpp
    core/font.cpp
    core/paths.cpp

    components/camera.cpp
    components/relation.cpp

    rendering/shapes.cpp

    serializers/components.cpp

    asset/assets.cpp
    asset/asset_handle.cpp
)
add_executable(kione2D::tests ALIAS K2_tests)

target_precompile_headers(K2_tests PRIVATE
    <iostream>
    <memory>
    <vector>
    <string>
    <unordered_map>
    <cstdint>
    <cmath>
    <algorithm>
    <stdexcept>
    <type_traits>
    <concepts>
    <format>
)

target_compile_definitions(K2_tests PRIVATE
    K2_TEST_FONT="${PROJECT_SOURCE_DIR}/player/res/fonts/NotoSans-Regular.ttf")

target_link_libraries(K2_tests
    kione2D::kione2D
    Catch2::Catch2WithMain
)

# Configure CMake to register this as a test
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)
catch_discover_tests(K2_tests)

set_target_properties(K2_tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 23
)
target_compile_options(K2_tests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)
