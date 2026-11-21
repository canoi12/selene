# ---------------------------------------------------------------------------
# selene runtime: core sources + one block per module, plus the front-end
# executables.  Feature detection lives in cmake/Check*.cmake and each module
# owns the sources it contributes.
# ---------------------------------------------------------------------------

# Shared include roots.  Kept directory-wide because the cross-cutting headers
# (include/common.h, include/selene.h, include/modules/renderer.h) pull glad,
# cglm, SDL and Vulkan from almost every translation unit.
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third
    ${CMAKE_SOURCE_DIR}/third/glad/include
    ${CMAKE_SOURCE_DIR}/third/cglm/include)

message("CMakeLibraryPath: ${CMAKE_LIBRARY_PATH}")
link_directories("${CMAKE_LIBRARY_PATH}")

# --- Graphics backend (desktop GL / Android GLES2 / Emscripten GLES3) ------
# Only the probe that matches the target platform is included.
if (ANDROID)
    include(CheckGLES2)
elseif (DEFINED EMSCRIPTEN)
    include(CheckGLES3)
elseif (NINTENDO_SWITCH)
    # Switch: GL comes from SDL2 (EGL/GLES over libnx or the NintendoSDK), so
    # there is no desktop OpenGL package to find. Normalize the platform macro
    # used by include/platforms.h (NintendoClang does not define __SWITCH__).
    add_definitions(-D__SWITCH__)
else ()
    include(CheckOpenGL)
endif ()

# --- Lua runtime -----------------------------------------------------------
include(CheckLua)

# --- SDL -------------------------------------------------------------------
include(CheckSDL)
include_directories(${SELENE_SDL_INCLUDE_DIRS})

# --- Vulkan (optional) -----------------------------------------------------
if (SELENE_NO_VULKAN)
    message("Disabling support for Vulkan")
    add_definitions(-DSELENE_NO_VULKAN=1)
else ()
    include(CheckVulkan)
endif ()

# Link interface shared by the shared library, the executable and the editor.
set(LIBRARIES ${LUA_LIB} ${OPENGL_LIBRARIES} cglm miniz ${SELENE_OPENGL_LIBRARY})
if (NOT DEFINED EMSCRIPTEN)
    list(APPEND LIBRARIES glad)
endif ()
if (SELENE_VULKAN_LIBRARY)
    list(APPEND LIBRARIES ${SELENE_VULKAN_LIBRARY})
endif ()
list(APPEND LIBRARIES ${SELENE_SDL_LIBRARIES})

# --- Core sources ----------------------------------------------------------
# main.c is the runtime front-end: it belongs to the executable only.
file(GLOB SELENE_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
list(REMOVE_ITEM SELENE_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

# Helper used by the module CMakeLists to attach their sources to whichever
# selene library flavor exists (static everywhere but MSVC, shared everywhere
# but Emscripten).  Callers pass absolute paths.
function(selene_add_module_sources)
    foreach (_selene_target ${PROJECT_NAME}Static ${PROJECT_NAME}Shared)
        if (TARGET ${_selene_target})
            target_sources(${_selene_target} PRIVATE ${ARGN})
        endif ()
    endforeach ()
endfunction()

## Static Library (not built with MSVC, which only produces the DLL)
if (NOT MSVC)
    add_library(${PROJECT_NAME}Static STATIC ${SELENE_CORE_SOURCES})
    set_target_properties(${PROJECT_NAME}Static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    if (NOT DEFINED NINTENDO_SWITCH)
        target_link_libraries(${PROJECT_NAME}Static m)
    endif()
    # libdl exists on Unix-likes only (not on Windows nor on the NintendoSDK).
    if (NOT WIN32 AND NOT NINTENDO_SWITCH)
        target_link_libraries(${PROJECT_NAME}Static dl)
    endif ()
endif ()

## Shared Library
# Not produced for Emscripten nor for the Nintendo Switch: neither has a
# dynamic linker, and the NintendoSDK linker rejects undefined symbols in
# shared objects (--no-allow-shlib-undefined). The static library + the
# executable are all that is needed there.
if (NOT DEFINED EMSCRIPTEN AND NOT NINTENDO_SWITCH)
    add_library(${PROJECT_NAME}Shared SHARED ${SELENE_CORE_SOURCES})
    target_link_libraries(${PROJECT_NAME}Shared ${LIBRARIES})
    if (MSVC)
        target_compile_definitions(${PROJECT_NAME}Shared PRIVATE -DSELENE_EXPORT_DLL)
        target_link_libraries(${PROJECT_NAME}Shared d3d11 d3dcompiler dxgi)
    endif ()
    if (HAS_LUA51)
        target_link_libraries(${PROJECT_NAME}Shared lua51)
    endif ()
    if (DEFINED MINGW)
        target_link_libraries(${PROJECT_NAME}Shared mingw32 opengl32 glu32 -mwindows)
    elseif (ANDROID)
        target_link_libraries(${PROJECT_NAME}Shared android log)
    endif ()
    set_target_properties(${PROJECT_NAME}Shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif ()

# --- Modules ---------------------------------------------------------------
add_subdirectory(modules)

# Alias used by the front-ends and the installer.
if (TARGET ${PROJECT_NAME}Shared)
    add_library(${PROJECT_NAME}Lib ALIAS ${PROJECT_NAME}Shared)
else ()
    add_library(${PROJECT_NAME}Lib ALIAS ${PROJECT_NAME}Static)
endif ()

# --- Runtime front-end -----------------------------------------------------
if (NOT ANDROID)
    add_executable(${PROJECT_NAME}Bin ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
    target_link_libraries(${PROJECT_NAME}Bin "${PROJECT_NAME}Lib" ${LIBRARIES})
    if (MSVC)
        target_compile_definitions(${PROJECT_NAME}Bin PRIVATE -DSELENE_IMPORT_DLL)
    endif ()
    if (HAS_LUA51)
        target_link_libraries(${PROJECT_NAME}Bin lua51)
    endif ()
    if (WIN32)
        target_sources(${PROJECT_NAME}Bin PRIVATE "${CMAKE_SOURCE_DIR}/misc/cross/windres/ICON.rc")
    endif ()
    set_target_properties(${PROJECT_NAME}Bin PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    if (DEFINED EMSCRIPTEN)
        message("Emscripten options")
        set_target_properties(${PROJECT_NAME}Bin PROPERTIES SUFFIX .html)
        target_link_options(${PROJECT_NAME}Bin PRIVATE -sUSE_SDL=2 -sMAIN_MODULE -sALLOW_MEMORY_GROWTH -sFORCE_FILESYSTEM -sMAX_WEBGL_VERSION=2 --shell-file ${CMAKE_SOURCE_DIR}/misc/cross/emscripten/index.html)
    endif ()
    if (DEFINED MINGW)
        target_link_libraries(${PROJECT_NAME}Bin mingw32 opengl32 glu32 -mwindows)
    endif ()
else ()
    add_library(${PROJECT_NAME}Bin SHARED ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
    set_target_properties(${PROJECT_NAME}Bin PROPERTIES OUTPUT_NAME main)
    target_link_libraries(${PROJECT_NAME}Bin "${PROJECT_NAME}Lib" ${LIBRARIES})
    target_link_libraries(${PROJECT_NAME}Bin android log)
endif ()

# --- Editor front-end (desktop only) ---------------------------------------
if (SELENE_BUILD_EDITOR AND NOT ANDROID AND NOT DEFINED EMSCRIPTEN AND NOT NINTENDO_SWITCH)
    add_subdirectory(editor)
endif ()
