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
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/res/fs.glsl ${CMAKE_BINARY_DIR}/bin/res COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/res/vs.glsl ${CMAKE_BINARY_DIR}/bin/res COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/res/texture.jpg ${CMAKE_BINARY_DIR}/bin/res COPYONLY)
