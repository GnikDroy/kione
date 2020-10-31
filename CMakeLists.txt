cmake_minimum_required(VERSION 3.18)

project(
    kione2D
    VERSION 0.1
    LANGUAGES CXX)


# Set the default build type
set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    if(BUILD_DOCS)
      find_package(Doxygen)
      message(STATUS "BUILD_DOCS is enabled")
      if(Doxygen_FOUND)
          add_subdirectory(docs)
      endif()
    endif()

    # Set cmake module path
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

endif()

Include(FetchContent)

# Setup EnTT
function(entt_build)
    set(BUILD_TESTING OFF)
    set(BUILD_DOCS OFF)
    FetchContent_Declare(
        EnTT
        GIT_REPOSITORY https://github.com/skypjack/entt
        GIT_TAG        v3.5.2)

    FetchContent_MakeAvailable(EnTT)
endfunction()
entt_build()

# Setup Catch 2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1)

FetchContent_MakeAvailable(Catch2)

# Setup spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG        v1.8.1)

FetchContent_MakeAvailable(spdlog)

# Setup fmtlib
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        7.0.3)

FetchContent_MakeAvailable(fmtlib)

# Setup glfw
function(glfw_build)
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw
      GIT_TAG        3.3.2)

    FetchContent_MakeAvailable(glfw)
endfunction()
glfw_build()


# Setup glad
function(glad_build)
    set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile")
    set(GLAD_API "gl=" CACHE STRING "API type/version pairs, like \"gl=3.2,gles=\", no version means latest")
    set(GLAD_GENERATOR "c" CACHE STRING "Language to generate the binding for")
    FetchContent_Declare(
      glad
      GIT_REPOSITORY https://github.com/Dav1dde/glad
      GIT_TAG        v0.1.34)

    FetchContent_MakeAvailable(glad)
endfunction()
glad_build()


# Setup imgui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.78
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp)
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/examples/)
  target_compile_definitions(imgui PUBLIC GL_GLEXT_PROTOTYPES=1)
endif()


# Add our source files
add_subdirectory(src)

# Testing only available if this is the main app
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
   AND BUILD_TESTING)
  message(STATUS "BUILD_TESTING is ENABLED.")
  add_subdirectory(tests)
  add_subdirectory(sandbox)
endif()