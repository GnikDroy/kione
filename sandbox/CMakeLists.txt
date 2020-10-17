add_executable(
  sandbox
  main.cpp
)

target_include_directories(sandbox PRIVATE ${PROJECT_SOURCE_DIR}/include) 
target_link_libraries(sandbox kione2D)

set_target_properties(sandbox
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)
target_compile_options(sandbox PRIVATE -Wall -Wextra -pedantic -Werror)
