find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(
  tests
  main.cpp
)

target_include_directories(tests PRIVATE ${PROJECT_SOURCE_DIR}/include) 
target_link_libraries(tests EnTT OpenGL::OpenGL SDL2::SDL2 imgui fmt::fmt Catch2::Catch2)
target_link_libraries(tests ${SDL2_IMAGE_LIBRARIES}
                              ${SDL2_TTF_LIBRARIES}
                              ${SDL2_MIXER_LIBRARIES})

set_target_properties(tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)
target_compile_options(tests PRIVATE -Wall -Wextra -pedantic -Werror)
