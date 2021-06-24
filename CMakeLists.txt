cmake_minimum_required(VERSION 3.17)

project(
    kione2D
    VERSION 0.1
    LANGUAGES CXX C)


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

    if(K2_BUILD_DOCS)
      find_package(Doxygen)
      message(STATUS "Building k2 docs.")
      if(Doxygen_FOUND)
          add_subdirectory(docs)
      endif()
    endif()

    # Set cmake module path
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

endif()

Include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# Setup EnTT
function(entt_build)
    set(ENTT_BUILD_TESTING   OFF CACHE INTERNAL "" FORCE)
    set(ENTT_BUILD_DOCS      OFF CACHE INTERNAL "" FORCE)
    FetchContent_Declare(
        EnTT
        GIT_REPOSITORY https://github.com/skypjack/entt
        GIT_TAG        master)

    FetchContent_MakeAvailable(EnTT)
endfunction()
entt_build()

# Setup Catch 2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1
  GIT_SHALLOW 1
  GIT_PROGRESS 1
  )

FetchContent_MakeAvailable(Catch2)

# Setup spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG        v1.8.1
  GIT_SHALLOW 1
  GIT_PROGRESS 1
  )

FetchContent_MakeAvailable(spdlog)

# Setup fmtlib
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        7.0.3
  GIT_SHALLOW 1
  GIT_PROGRESS 1
  )

FetchContent_MakeAvailable(fmtlib)

# Setup glm
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG        0.9.9.8
        GIT_SHALLOW 1
        GIT_PROGRESS 1
)

FetchContent_MakeAvailable(glm)

# Setup glfw
function(glfw_build)
    set(GLFW_BUILD_EXAMPLES  OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS     OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS      OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL         OFF CACHE INTERNAL "Generate installation target")
    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw
      GIT_TAG        3.3.2
      GIT_SHALLOW 1
      GIT_PROGRESS 1
    )

    FetchContent_MakeAvailable(glfw)
endfunction()
glfw_build()

# Setup glad
function(glad_build)
    set(GLAD_PROFILE    "core"  CACHE INTERNAL "")
    set(GLAD_API        "gl="   CACHE INTERNAL "")
    set(GLAD_GENERATOR  "c"     CACHE INTERNAL "")
    FetchContent_Declare(
      glad
      GIT_REPOSITORY https://github.com/Dav1dde/glad
      GIT_TAG        v0.1.34
      GIT_SHALLOW 1
      GIT_PROGRESS 1
    )

    FetchContent_MakeAvailable(glad)
endfunction()
glad_build()


# Setup imgui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.83
  GIT_SHALLOW 1
  GIT_PROGRESS 1
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
  set_target_properties(imgui
          PROPERTIES
          ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
          LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
          CXX_STANDARD 17
          )
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends/)
  target_link_libraries(imgui glfw glad)
endif()

# Setup stb
function(stb_build)
    FetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb
            GIT_TAG        master
            GIT_SHALLOW 1
            GIT_PROGRESS 1
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
    )
    FetchContent_GetProperties(stb)
    if(NOT stb_POPULATED)
        FetchContent_Populate(stb)
    endif()
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endfunction()
stb_build()

# Setup assimp
function(assimp_build)
#    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Generation of shared libs. Set this to OFF to get static lib" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "If supplementary tools for Assimp are built in addition to the library" FORCE)
    set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "If the test suite for Assimp is built in addition to the library." FORCE)
    # NOTE: This might be useful later.
    set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "Disable Assimp's export functionality" FORCE)

    FetchContent_Declare(
            assimp
            GIT_REPOSITORY https://github.com/assimp/assimp
            GIT_TAG        master
            GIT_SHALLOW 1
            GIT_PROGRESS 1
    )
    FetchContent_GetProperties(assimp)
    if(NOT assimp_POPULATED)
        FetchContent_Populate(assimp)
    endif()

    if(MSVC)
        # Assimp suppress array equality operator deprecation and other warnings.
        set(CMAKE_CXX_FLAGS "/wd4530 /wd5056")
    endif()
    add_subdirectory(${assimp_SOURCE_DIR} ${assimp_BINARY_DIR})
endfunction()
assimp_build()


# Export compile commands for all of our source files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add our source files
add_subdirectory(src)

# Testing only available is stated explicitly
if(K2_BUILD_TESTS)
  message(STATUS "Building k2 tests.")
  add_subdirectory(tests)
endif()

# Setup sandbox build
set(K2_BUILD_SANDBOX ON CACHE BOOL "Enable building sandbox")
if(K2_BUILD_SANDBOX)
    message(STATUS "Building k2 sandbox application.")
    add_subdirectory(sandbox)
endif()

# Setup sandbox build
set(K2_BUILD_SANDBOX2D ON CACHE BOOL "Enable building sandbox2D")
if(K2_BUILD_SANDBOX2D)
    message(STATUS "Building k2 sandbox application.")
    add_subdirectory(sandbox2D)
endif()
