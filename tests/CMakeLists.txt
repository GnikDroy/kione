find_package(OpenGL REQUIRED)

add_executable(
  tests
  main.cpp
  resource_container.cpp
  vector.cpp
)

target_include_directories(tests PRIVATE ${PROJECT_SOURCE_DIR}/include) 
target_precompile_headers(tests PRIVATE
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
target_link_libraries(tests kione2D fmt::fmt Catch2::Catch2)

set_target_properties(tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)
target_compile_options(tests PRIVATE -Wall -Wextra -pedantic -Werror)
