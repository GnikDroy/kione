find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(
  kione2D
  main.cpp
)

target_include_directories(kione2D PRIVATE ${PROJECT_SOURCE_DIR}/include) 
target_link_libraries(kione2D EnTT OpenGL::OpenGL SDL2::SDL2 imgui)
target_link_libraries(kione2D ${SDL2_IMAGE_LIBRARIES}
                              ${SDL2_TTF_LIBRARIES}
                              ${SDL2_MIXER_LIBRARIES})

set_target_properties(kione2D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)
target_compile_options(kione2D PRIVATE -Wall -Wextra -pedantic -Werror)
