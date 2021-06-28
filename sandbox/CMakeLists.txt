add_executable(
  K2_sandbox
  main.cpp
)
add_executable(kione2D::sandbox ALIAS K2_sandbox)

target_include_directories(K2_sandbox PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(K2_sandbox kione2D::kione2D)

set_target_properties(K2_sandbox
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)

if (WIN32)
    set_target_properties(K2_sandbox
            PROPERTIES
            WIN32_EXECUTABLE true)
endif()

# /wd4103 due to a msvc compiler bug
# see https://developercommunity.visualstudio.com/t/warning-c4103-in-visual-studio-166-update/1057589
target_compile_options(K2_sandbox PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /std:c++latest /wd4201 /wd4103>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)

add_custom_command(TARGET K2_sandbox PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:K2_sandbox>/res/)