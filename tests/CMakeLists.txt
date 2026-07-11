add_executable(
    K2_tests

    core/resource_container.cpp
    core/resource_manager.cpp
    core/scene_loader.cpp
    core/vector.cpp
    core/utils.cpp
    core/imgui_theme.cpp
    core/fnv.cpp
    
    asset/assets.cpp
    asset/asset_handle.cpp
)
add_executable(kione2D::tests ALIAS K2_tests)

target_include_directories(K2_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
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
