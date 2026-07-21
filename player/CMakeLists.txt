add_executable(
  player
  main.cpp
)
add_executable(kione2D::player ALIAS player)

target_link_libraries(player kione2D::kione2D)

set_target_properties(player
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 23
)

if (WIN32)
    set_target_properties(player
            PROPERTIES
            WIN32_EXECUTABLE true)
endif()

target_compile_options(player PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest /wd4201>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

add_custom_command(TARGET player POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:player>/res/
    COMMENT "COPY res directory to build"
)
