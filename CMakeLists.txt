cmake_minimum_required(VERSION 3.16)
project(
    kione2D
    VERSION 0.1
    LANGUAGES CXX C
)


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(BUILD_SHARED_LIBS OFF)

# sol2 is very template heavy
if(MSVC)
    add_compile_options(/bigobj)
endif()

# Set the default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(default_build_type "Debug")
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(FETCHCONTENT_QUIET FALSE)

    set(CMAKE_CXX_STANDARD 23)
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

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Setup EnTT
function(entt_build)
    set(ENTT_BUILD_TESTING   OFF CACHE INTERNAL "" FORCE)
    set(ENTT_BUILD_DOCS      OFF CACHE INTERNAL "" FORCE)
    FetchContent_Declare(
        EnTT
        GIT_REPOSITORY https://github.com/skypjack/entt
        GIT_TAG        v3.16.0
    )

    FetchContent_MakeAvailable(EnTT)
endfunction()
entt_build()

# Setup yaml
function(yaml_build)
    set(YAML_CPP_BUILD_TESTS OFF)
    FetchContent_Declare(
            yaml-cpp
            GIT_REPOSITORY https://github.com/jasonbeach/yaml-cpp
            GIT_TAG dec18b8d7f25bed77dd3bee44d9c3874437ae7a7 # cmake_install_fix
    )
    FetchContent_MakeAvailable(yaml-cpp)
endfunction()
yaml_build()

# Setup Catch 2
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.15.2
    GIT_SHALLOW 1
    GIT_PROGRESS 1
)

FetchContent_MakeAvailable(Catch2)

# Setup spdlog
function(spdlog_build)
    set(SPDLOG_USE_STD_FORMAT ON CACHE INTERNAL "spdlog uses std::format" FORCE)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG        v1.17.0
        GIT_SHALLOW 1
        GIT_PROGRESS 1
    )
    FetchContent_MakeAvailable(spdlog)
endfunction()
spdlog_build()


# Setup glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        1.0.3
    GIT_SHALLOW 1
    GIT_PROGRESS 1
)

FetchContent_MakeAvailable(glm)
# glm 1.x gates the gtx/ extension headers behind this definition
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)

# Setup glfw
function(glfw_build)
    set(GLFW_BUILD_EXAMPLES  OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS     OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS      OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL         OFF CACHE INTERNAL "Generate installation target")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG        3.4
        GIT_SHALLOW 1
        GIT_PROGRESS 1
    )

    FetchContent_MakeAvailable(glfw)
endfunction()
glfw_build()

# Setup nfd
FetchContent_Declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
    GIT_TAG        v1.3.0
    GIT_SHALLOW 1
    GIT_PROGRESS 1
)
FetchContent_MakeAvailable(nfd)

# Setup glad
function(glad_build)
    set(GLAD_PROFILE    "core"  CACHE INTERNAL "")
    set(GLAD_API        "gl="   CACHE INTERNAL "")
    set(GLAD_GENERATOR  "c"     CACHE INTERNAL "")
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad
        GIT_TAG        v0.1.36
        GIT_SHALLOW 1
        GIT_PROGRESS 1
    )

    FetchContent_MakeAvailable(glad)
endfunction()
glad_build()


# Setup Icon Fonts (Font Awesome)
FetchContent_Declare(
    iconFontCppHeaders
    GIT_REPOSITORY https://github.com/juliettef/IconFontCppHeaders
    GIT_TAG        210b5a399a64270674560d633638952d1e8d804d
    GIT_PROGRESS 1
)

FetchContent_GetProperties(IconFontCppHeaders)
if(NOT IconFontCppHeaders_POPULATED)
    FetchContent_Populate(IconFontCppHeaders)
    add_library(IconFontCppHeaders INTERFACE)
    add_library(IconFontCppHeaders::IconFontCppHeaders ALIAS IconFontCppHeaders)
    # Note that {lowercasename_SOURCE_DIR} is populated
    target_include_directories(IconFontCppHeaders INTERFACE ${iconfontcppheaders_SOURCE_DIR})
endif()

# Setup imgui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        ca49eff3980443a97c470e09fe55b1740cfb9584 # docking
    GIT_PROGRESS 1
)

FetchContent_GetProperties(ImGui)
if(NOT ImGui_POPULATED)
    FetchContent_Populate(ImGui)
    add_library(ImGui
        ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    add_library(ImGui::ImGui ALIAS ImGui)
    set_target_properties(ImGui
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        CXX_STANDARD 17
    )
    target_include_directories(ImGui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends/ ${imgui_SOURCE_DIR}/misc/cpp/)
    target_link_libraries(ImGui glfw glad)
endif()

# Setup ImGuizmo
FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo
    GIT_TAG        dc25afb98bc3ebe00dfc9a23ba7235fead2ccb1d # master
    GIT_PROGRESS 1
)

FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
    add_library(ImGuizmo ${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp)
    add_library(ImGuizmo::ImGuizmo ALIAS ImGuizmo)
    set_target_properties(ImGuizmo
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        CXX_STANDARD 17
    )
    target_include_directories(ImGuizmo PUBLIC ${imguizmo_SOURCE_DIR}/src)
    target_link_libraries(ImGuizmo ImGui::ImGui)
endif()

# Setup stb
function(stb_build)
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb
        GIT_TAG        31c1ad37456438565541f4919958214b6e762fb4 # master
        GIT_PROGRESS 1
    )
    FetchContent_GetProperties(stb)
    if(NOT stb_POPULATED)
        FetchContent_Populate(stb)
    endif()
    add_library(stb INTERFACE)
    add_library(stb::stb ALIAS stb)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endfunction()
stb_build()

# Setup miniaudio
function(miniaudio_build)
    FetchContent_Declare(
        miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio
        GIT_TAG        0.11.25
        GIT_SHALLOW 1
        GIT_PROGRESS 1
    )
    FetchContent_GetProperties(miniaudio)
    if(NOT miniaudio_POPULATED)
        FetchContent_Populate(miniaudio)
    endif()
    add_library(miniaudio INTERFACE)
    add_library(miniaudio::miniaudio ALIAS miniaudio)
    target_include_directories(miniaudio INTERFACE ${miniaudio_SOURCE_DIR})
endfunction()
miniaudio_build()

# Setup lua
function(lua_build)
    FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/lua/lua
        GIT_TAG        v5.4.8
        GIT_SHALLOW 1
        GIT_PROGRESS 1
    )
    FetchContent_GetProperties(lua)
    if(NOT lua_POPULATED)
        FetchContent_Populate(lua)
    endif()
    add_library(lua STATIC
        ${lua_SOURCE_DIR}/lapi.c
        ${lua_SOURCE_DIR}/lcode.c
        ${lua_SOURCE_DIR}/lctype.c
        ${lua_SOURCE_DIR}/ldebug.c
        ${lua_SOURCE_DIR}/ldo.c
        ${lua_SOURCE_DIR}/ldump.c
        ${lua_SOURCE_DIR}/lfunc.c
        ${lua_SOURCE_DIR}/lgc.c
        ${lua_SOURCE_DIR}/llex.c
        ${lua_SOURCE_DIR}/lmem.c
        ${lua_SOURCE_DIR}/lobject.c
        ${lua_SOURCE_DIR}/lopcodes.c
        ${lua_SOURCE_DIR}/lparser.c
        ${lua_SOURCE_DIR}/lstate.c
        ${lua_SOURCE_DIR}/lstring.c
        ${lua_SOURCE_DIR}/ltable.c
        ${lua_SOURCE_DIR}/ltm.c
        ${lua_SOURCE_DIR}/lundump.c
        ${lua_SOURCE_DIR}/lvm.c
        ${lua_SOURCE_DIR}/lzio.c
        ${lua_SOURCE_DIR}/lauxlib.c
        ${lua_SOURCE_DIR}/lbaselib.c
        ${lua_SOURCE_DIR}/lcorolib.c
        ${lua_SOURCE_DIR}/ldblib.c
        ${lua_SOURCE_DIR}/liolib.c
        ${lua_SOURCE_DIR}/lmathlib.c
        ${lua_SOURCE_DIR}/loadlib.c
        ${lua_SOURCE_DIR}/loslib.c
        ${lua_SOURCE_DIR}/lstrlib.c
        ${lua_SOURCE_DIR}/ltablib.c
        ${lua_SOURCE_DIR}/lutf8lib.c
        ${lua_SOURCE_DIR}/linit.c
    )
    add_library(lua::lua ALIAS lua)
    target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})
endfunction()
lua_build()

function(sol2_build)
    FetchContent_Declare(
        sol2
        GIT_REPOSITORY https://github.com/ThePhD/sol2
        GIT_TAG        c1f95a773c6f8f4fde8ca3efe872e7286afe4444 # develop
        GIT_PROGRESS 1
    )
    FetchContent_MakeAvailable(sol2)
endfunction()
sol2_build()

# Setup tinyobjloader
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
    GIT_TAG        45636bdcef1a4fec140346b90c0b50bf0bc3e23b # release
    GIT_PROGRESS 1
)
FetchContent_MakeAvailable(tinyobjloader)

# Export compile commands for all of our source files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add our source files
add_subdirectory(src)

# Testing only available if stated explicitly
set(K2_BUILD_TESTS ON CACHE BOOL "Enable building k2 tests")
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

# Setup player build
set(K2_BUILD_PLAYER ON CACHE BOOL "Enable building the player")
if(K2_BUILD_PLAYER)
    message(STATUS "Building k2 player application.")
    add_subdirectory(player)
endif()

# Setup editor build
set(K2_BUILD_EDITOR ON CACHE BOOL "Enable building editor")
if(K2_BUILD_EDITOR)
    message(STATUS "Building k2 editor.")
    add_subdirectory(editor)
endif()
