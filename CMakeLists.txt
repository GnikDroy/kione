cmake_minimum_required(VERSION 3.16)
project(
    kione2D
    VERSION 0.1
    LANGUAGES CXX C
)


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

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

# Setup EnTT
function(entt_build)
    set(ENTT_BUILD_TESTING   OFF CACHE INTERNAL "" FORCE)
    set(ENTT_BUILD_DOCS      OFF CACHE INTERNAL "" FORCE)
    FetchContent_Declare(
        EnTT
        GIT_REPOSITORY https://github.com/skypjack/entt
        GIT_TAG        v3.14.0
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
            GIT_TAG cmake_install_fix
    )
    FetchContent_MakeAvailable(yaml-cpp)
endfunction()
yaml_build()

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
function(spdlog_build)
    set(SPDLOG_USE_STD_FORMAT ON CACHE INTERNAL "spdlog uses std::format" FORCE)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG        v1.15.0
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

# Setup nfd
FetchContent_Declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
    GIT_TAG        v1.2.1
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
        GIT_TAG        v0.1.34
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
    GIT_TAG        main
    GIT_SHALLOW 1
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
    GIT_TAG        origin/docking
    GIT_SHALLOW 1
    GIT_PROGRESS 1
)

FetchContent_GetProperties(ImGui)
if(NOT ImGui_POPULATED)
    FetchContent_Populate(ImGui)
    add_library(ImGui
        ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    add_library(ImGui::ImGui ALIAS ImGui)
    set_target_properties(ImGui
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        CXX_STANDARD 17
    )
    target_include_directories(ImGui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends/)
    target_link_libraries(ImGui glfw glad)
endif()

# Setup stb
function(stb_build)
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb
        GIT_TAG        master
        GIT_SHALLOW 1
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

# Setup lua
function(lua_build)
    FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/lua/lua
        GIT_TAG        v5.4.3
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
        GIT_TAG        v3.2.2
        GIT_SHALLOW 1
        GIT_PROGRESS 1
    )
    FetchContent_MakeAvailable(sol2)
endfunction()
sol2_build()

# Setup assimp
function(assimp_build)
    # set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Generation of shared libs. Set this to OFF to get static lib" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "If supplementary tools for Assimp are built in addition to the library" FORCE)
    set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "If the test suite for Assimp is built in addition to the library." FORCE)
    set(ASSIMP_INSTALL_PDB OFF CACHE INTERNAL "Install pdb" FORCE)

    set(ASSIMP_INSTALL ON CACHE INTERNAL "Don't install assimp" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE INTERNAL "Enable all importers by default" FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE INTERNAL "Enable OBJ importer" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE INTERNAL "Enable FBX importer" FORCE)
    set(ASSIMP_BUILD_BLEND_IMPORTER ON CACHE INTERNAL "Enable BLEND importer" FORCE)
    set(ASSIMP_BUILD_COLLADA_IMPORTER ON CACHE INTERNAL "Enable COLLADA importer" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTER OFF CACHE INTERNAL "Enable all importer" FORCE)

    set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "Disable Assimp's export functionality" FORCE)

    FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp
        GIT_TAG        v5.4.3
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

# Setup sandbox2D build
set(K2_BUILD_SANDBOX2D ON CACHE BOOL "Enable building sandbox2D")
if(K2_BUILD_SANDBOX2D)
    message(STATUS "Building k2 sandbox2D application.")
    add_subdirectory(sandbox2D)
endif()

# Setup editor build
set(K2_BUILD_EDITOR ON CACHE BOOL "Enable building editor")
if(K2_BUILD_EDITOR)
    message(STATUS "Building k2 editor.")
    add_subdirectory(editor)
endif()
