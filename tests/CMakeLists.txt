add_executable(
  K2_tests
  main.cpp
  resource_container.cpp
  vector.cpp
  utils.cpp
  assets.cpp
  imgui_theme.cpp
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

  [["fmt/core.h"]]
  [["fmt/ranges.h"]]
  [["fmt/format.h"]]
)
target_link_libraries(K2_tests
    kione2D::kione2D
    fmt::fmt
    Catch2::Catch2)

set_target_properties(K2_tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)
target_compile_options(K2_tests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)
