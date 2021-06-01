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

if (WIN32)
    set_target_properties(sandbox
            PROPERTIES
            WIN32_EXECUTABLE true)
endif()

target_compile_options(sandbox PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest /wd4201>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)

add_custom_command(TARGET sandbox PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:sandbox>/res/)