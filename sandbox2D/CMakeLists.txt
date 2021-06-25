add_executable(
  sandbox2D
  main.cpp
)
add_executable(kione2D::sandbox2D ALIAS sandbox2D)

target_include_directories(sandbox2D PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(sandbox2D kione2D::kione2D)

set_target_properties(sandbox2D
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)

if (WIN32)
    set_target_properties(sandbox2D
            PROPERTIES
            WIN32_EXECUTABLE true)
endif()

# /wd4103 due to a msvc compiler bug
# see https://developercommunity.visualstudio.com/t/warning-c4103-in-visual-studio-166-update/1057589
target_compile_options(sandbox2D PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest /wd4201 /wd4103>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)

add_custom_command(TARGET sandbox2D PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:sandbox2D>/res/)