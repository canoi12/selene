#[=======================================================================[.rst:
FindCustomSDL3
--------------

Locates SDL3 for selene.  Same interface as ``FindCustomSDL2``:

1. Config package (``SDL3_DIR``, or one found inside ``SDL_DIR``).
2. Explicit ``SDL_DIR`` without a cmake package (manual lookup).
3. Regular ``find_package(SDL3 CONFIG)``.
4. Manual lookup with default search paths.
5. Build from source (``SDL_DIR``/``SDL3_SOURCE_DIR``) or force it with
   ``USE_SDL_SOURCE``.

Input variables
^^^^^^^^^^^^^^^

``SDL_DIR``
  Root of an SDL3 installation, precompiled package or source tree.  It is
  only used when it actually looks like SDL3 (``include/SDL3/SDL.h`` or a
  ``SDL3Config.cmake``), so an ``SDL_DIR`` pointing to SDL2 is ignored.
  Defaults to ``${CACHE_DIR}/SDL3/{MSVC,MINGW,source}``.

``SDL3_DIR``
  Standard CMake package directory; takes precedence over ``SDL_DIR``.

``SDL3_USE_STATIC``
  Prefer ``SDL3::SDL3-static``.

``USE_SDL_SOURCE``
  Force building SDL3 from source.

Result variables
^^^^^^^^^^^^^^^^

``SDL3_FOUND``, ``SDL3_INCLUDE_DIRS``, ``SDL3_LIBRARIES``, ``SDL3_DLL``

Imported targets
^^^^^^^^^^^^^^^^

``SDL3::SDL3`` (and ``SDL3::SDL3main`` on Windows when available)
#]=======================================================================]

include_guard(GLOBAL)

# --- Default SDL3 root -----------------------------------------------------
if (MSVC)
    set(_sdl3_default_dir "${CACHE_DIR}/SDL3/MSVC")
elseif (MINGW)
    set(_sdl3_default_dir "${CACHE_DIR}/SDL3/MINGW")
else ()
    set(_sdl3_default_dir "${CACHE_DIR}/SDL3/source")
endif ()

file(TO_CMAKE_PATH "${_sdl3_default_dir}" _sdl3_default_root)
string(REGEX REPLACE "/+$" "" _sdl3_default_root "${_sdl3_default_root}")

set(_sdl3_root "")

# SDL_DIR is shared with FindCustomSDL2: only accept it when it is SDL3.
if (SDL_DIR)
    file(TO_CMAKE_PATH "${SDL_DIR}" _sdl3_sdl_dir)
    string(REGEX REPLACE "/+$" "" _sdl3_sdl_dir "${_sdl3_sdl_dir}")
    if (EXISTS "${_sdl3_sdl_dir}/include/SDL3/SDL.h" OR
        EXISTS "${_sdl3_sdl_dir}/SDL3Config.cmake" OR
        EXISTS "${_sdl3_sdl_dir}/cmake/SDL3Config.cmake" OR
        EXISTS "${_sdl3_sdl_dir}/lib/cmake/SDL3/SDL3Config.cmake")
        set(_sdl3_root "${_sdl3_sdl_dir}")
    endif ()
endif ()

if ("${_sdl3_root}" STREQUAL "")
    set(_sdl3_root "${_sdl3_default_dir}")
endif ()

file(TO_CMAKE_PATH "${_sdl3_root}" _sdl3_root)
string(REGEX REPLACE "/+$" "" _sdl3_root "${_sdl3_root}")

set(_sdl3_vendor_roots "")
file(GLOB _sdl3_vendor_roots
    "${_sdl3_root}/*-w64-mingw32"
    "${_sdl3_root}/*-linux-android")
if (DEFINED triple AND NOT "${triple}" STREQUAL "" AND
    EXISTS "${_sdl3_root}/${triple}")
    list(APPEND _sdl3_vendor_roots "${_sdl3_root}/${triple}")
endif ()
list(REMOVE_DUPLICATES _sdl3_vendor_roots)

set(_sdl3_search_roots "${_sdl3_root}")
list(APPEND _sdl3_search_roots ${_sdl3_vendor_roots})

# --- Source tree / legacy source dir ---------------------------------------
set(_sdl3_source_tree "")
foreach (_sdl3_candidate IN LISTS _sdl3_search_roots)
    if (EXISTS "${_sdl3_candidate}/CMakeLists.txt" AND
        EXISTS "${_sdl3_candidate}/include/SDL3/SDL.h")
        set(_sdl3_source_tree "${_sdl3_candidate}")
        break()
    endif ()
endforeach ()

if ("${_sdl3_source_tree}" STREQUAL "" AND SDL3_SOURCE_DIR AND
    EXISTS "${SDL3_SOURCE_DIR}/CMakeLists.txt" AND
    EXISTS "${SDL3_SOURCE_DIR}/include/SDL3/SDL.h")
    set(_sdl3_source_tree "${SDL3_SOURCE_DIR}")
endif ()

# --- Invalidate cached lookups when the root changes ----------------------
if (NOT DEFINED SELENE_SDL3_DIR_HINT)
    set(SELENE_SDL3_DIR_HINT "" CACHE INTERNAL
        "SDL_DIR value used by FindCustomSDL3")
endif ()

if (NOT "${SELENE_SDL3_DIR_HINT}" STREQUAL "${_sdl3_root}")
    unset(SDL3_INCLUDE_DIR CACHE)
    unset(SDL3_LIBRARY CACHE)
    unset(SDL3main_LIBRARY CACHE)
    unset(SDL3_DLL CACHE)
    set(SELENE_SDL3_DIR_HINT "${_sdl3_root}" CACHE INTERNAL
        "SDL_DIR value used by FindCustomSDL3")
endif ()

# --- Manual discovery ------------------------------------------------------
function(_selene_sdl3_discover no_default_path)
    set(_extra_args "")
    if (no_default_path)
        list(APPEND _extra_args NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    endif ()

    find_path(SDL3_INCLUDE_DIR
        NAMES SDL3/SDL.h
        HINTS ${SDL3_SEARCH_HINTS}
        ${_extra_args})
    find_library(SDL3_LIBRARY
        NAMES ${SDL3_LIBRARY_NAMES}
        HINTS ${SDL3_SEARCH_HINTS}
        ${_extra_args})

    if (WIN32)
        find_library(SDL3main_LIBRARY
            NAMES SDL3main
            HINTS ${SDL3_SEARCH_HINTS}
            ${_extra_args})
        find_file(SDL3_DLL
            NAMES SDL3.dll
            HINTS ${SDL3_SEARCH_HINTS}
            ${_extra_args})
    endif ()

    if (NOT SDL3_INCLUDE_DIR OR NOT SDL3_LIBRARY)
        return()
    endif ()

    set(SDL3_INCLUDE_DIRS "${SDL3_INCLUDE_DIR}" PARENT_SCOPE)

    if (TARGET SDL3::SDL3)
        set(SDL3_TARGET_FOUND TRUE PARENT_SCOPE)
        return()
    endif ()

    set(_shared FALSE)
    get_filename_component(_lib_name "${SDL3_LIBRARY}" NAME)
    if (SDL3_USE_STATIC)
        set(_shared FALSE)
    elseif (_lib_name MATCHES "\\.dll\\.a$" OR
            _lib_name MATCHES "\\.so(\\.[0-9.]+)?$" OR
            _lib_name MATCHES "\\.dylib$")
        set(_shared TRUE)
    elseif (WIN32 AND _lib_name MATCHES "\\.lib$")
        set(_shared TRUE)
    endif ()

    if (_shared)
        add_library(SDL3::SDL3 SHARED IMPORTED)
        if (SDL3_DLL)
            set_target_properties(SDL3::SDL3 PROPERTIES
                IMPORTED_LOCATION "${SDL3_DLL}"
                IMPORTED_IMPLIB "${SDL3_LIBRARY}")
        else ()
            set_target_properties(SDL3::SDL3 PROPERTIES
                IMPORTED_LOCATION "${SDL3_LIBRARY}")
        endif ()
    else ()
        add_library(SDL3::SDL3 STATIC IMPORTED)
        set_target_properties(SDL3::SDL3 PROPERTIES
            IMPORTED_LOCATION "${SDL3_LIBRARY}")
    endif ()
    set_target_properties(SDL3::SDL3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIRS}")

    if (WIN32 AND SDL3main_LIBRARY AND NOT TARGET SDL3::SDL3main)
        add_library(SDL3::SDL3main STATIC IMPORTED)
        set_target_properties(SDL3::SDL3main PROPERTIES
            IMPORTED_LOCATION "${SDL3main_LIBRARY}"
            INTERFACE_LINK_LIBRARIES SDL3::SDL3)
    endif ()

    set(SDL3_TARGET_FOUND TRUE PARENT_SCOPE)
endfunction()

set(SDL3_LIBRARY_NAMES SDL3 SDL3-static)
if (SDL3_USE_STATIC)
    set(SDL3_LIBRARY_NAMES SDL3-static SDL3)
endif ()

set(SDL3_SEARCH_HINTS "")
foreach (_sdl3_dir IN LISTS _sdl3_search_roots)
    if (NOT "${_sdl3_dir}" STREQUAL "${_sdl3_source_tree}")
        list(APPEND SDL3_SEARCH_HINTS
            "${_sdl3_dir}/include"
            "${_sdl3_dir}"
            "${_sdl3_dir}/lib"
            "${_sdl3_dir}/lib64"
            "${_sdl3_dir}/lib/x64"
            "${_sdl3_dir}/lib/x86"
            "${_sdl3_dir}/bin")
    endif ()
endforeach ()


# --- Resolution ------------------------------------------------------------
set(_sdl3_target "")

set(_sdl3_pkg_dir "")
if (SDL3_DIR)
    set(_sdl3_pkg_dir "${SDL3_DIR}")
endif ()

set(_sdl3_sdl_dir "not set")
if (DEFINED SDL_DIR)
    set(_sdl3_sdl_dir "${SDL_DIR}")
endif ()

if (USE_SDL_SOURCE)
    if ("${_sdl3_source_tree}" STREQUAL "")
        message(FATAL_ERROR
            "USE_SDL_SOURCE is ON but no SDL3 source tree was found.\n"
            "Point SDL_DIR (or SDL3_SOURCE_DIR) to an SDL3 source checkout.")
    endif ()
else ()
    # 1. Package directory given by the user / toolchain.
    find_package(SDL3 CONFIG QUIET)
    if (SDL3_DIR)
        set(_sdl3_pkg_dir "${SDL3_DIR}")
    endif ()

    if (TARGET SDL3::SDL3)
        set(_sdl3_target SDL3::SDL3)
    elseif (TARGET SDL3::SDL3-shared)
        set(_sdl3_target SDL3::SDL3-shared)
    elseif (TARGET SDL3::SDL3-static)
        set(_sdl3_target SDL3::SDL3-static)
    endif ()

    # 2. Config package inside SDL_DIR.
    if (NOT _sdl3_target AND EXISTS "${_sdl3_root}")
        set(_sdl3_config_paths "")
        foreach (_sdl3_dir IN LISTS _sdl3_search_roots)
            list(APPEND _sdl3_config_paths
                "${_sdl3_dir}"
                "${_sdl3_dir}/cmake"
                "${_sdl3_dir}/lib/cmake/SDL3"
                "${_sdl3_dir}/lib/${triple}/cmake/SDL3"
                "${_sdl3_dir}/share/cmake/SDL3")
        endforeach ()

        foreach (_sdl3_dir IN LISTS _sdl3_config_paths)
            if (EXISTS "${_sdl3_dir}/SDL3Config.cmake" OR
                EXISTS "${_sdl3_dir}/sdl3-config.cmake")
                set(SDL3_DIR "${_sdl3_dir}" CACHE PATH
                    "Directory containing SDL3Config.cmake")
                set(_sdl3_pkg_dir "${_sdl3_dir}")
                break()
            endif ()
        endforeach ()

        if (NOT "${_sdl3_pkg_dir}" STREQUAL "")
            find_package(SDL3 CONFIG QUIET)
        endif ()

        if (TARGET SDL3::SDL3)
            set(_sdl3_target SDL3::SDL3)
        elseif (TARGET SDL3::SDL3-shared)
            set(_sdl3_target SDL3::SDL3-shared)
        elseif (TARGET SDL3::SDL3-static)
            set(_sdl3_target SDL3::SDL3-static)
        endif ()
    endif ()

    # 3. Explicit SDL_DIR without cmake package.
    if (NOT _sdl3_target AND
        NOT "${_sdl3_root}" STREQUAL "${_sdl3_default_root}")
        _selene_sdl3_discover(TRUE)
        if (TARGET SDL3::SDL3)
            set(_sdl3_target SDL3::SDL3)
        endif ()
    endif ()

    # 4. System libraries without a cmake package.
    if (NOT _sdl3_target)
        _selene_sdl3_discover(FALSE)
        if (TARGET SDL3::SDL3)
            set(_sdl3_target SDL3::SDL3)
        endif ()
    endif ()
endif ()

# 5. Build from source.
if (NOT _sdl3_target AND NOT "${_sdl3_source_tree}" STREQUAL "")
    message(STATUS "SDL3: building from source: ${_sdl3_source_tree}")
    add_subdirectory("${_sdl3_source_tree}" "${CMAKE_BINARY_DIR}/_deps/sdl3")

    if (NOT TARGET SDL3::SDL3)
        if (TARGET SDL3-shared)
            add_library(SDL3::SDL3 ALIAS SDL3-shared)
        elseif (TARGET SDL3-static)
            add_library(SDL3::SDL3 ALIAS SDL3-static)
        endif ()
    endif ()
    if (WIN32 AND TARGET SDL3main AND NOT TARGET SDL3::SDL3main)
        add_library(SDL3::SDL3main ALIAS SDL3main)
    endif ()

    set(SDL3_INCLUDE_DIRS "${_sdl3_source_tree}/include")

    if (TARGET SDL3::SDL3)
        set(_sdl3_target SDL3::SDL3)
    endif ()
endif ()

if (NOT _sdl3_target)
    message(FATAL_ERROR
        "SDL3 not found.\n"
        "  SDL_DIR  = ${_sdl3_sdl_dir}\n"
        "  SDL3_DIR = ${_sdl3_pkg_dir}\n"
        "Hints:\n"
        "  * -DSDL_DIR=<path>  (precompiled package, installation prefix or source tree)\n"
        "  * -DSDL3_DIR=<path> (directory containing SDL3Config.cmake)\n"
        "  * -DUSE_SDL_SOURCE=ON with a valid SDL3 source tree")
endif ()

# --- Selection / outputs ---------------------------------------------------
if (SDL3_USE_STATIC AND TARGET SDL3::SDL3-static)
    set(_sdl3_target SDL3::SDL3-static)
endif ()

set(SDL3_FOUND TRUE)
set(CustomSDL3_FOUND TRUE)

set(SDL3_LIBRARIES "${_sdl3_target}")
if (WIN32 AND TARGET SDL3::SDL3main)
    list(INSERT SDL3_LIBRARIES 0 SDL3::SDL3main)
endif ()

if (WIN32 AND NOT SDL3_DLL AND TARGET ${_sdl3_target})
    get_target_property(_sdl3_location ${_sdl3_target} IMPORTED_LOCATION)
    get_target_property(_sdl3_implib ${_sdl3_target} IMPORTED_IMPLIB)
    if (_sdl3_location AND _sdl3_implib)
        set(SDL3_DLL "${_sdl3_location}")
    endif ()
endif ()

message(STATUS "SDL3: using ${_sdl3_target}")
if (SDL3_INCLUDE_DIRS)
    message(STATUS "SDL3: include dirs ${SDL3_INCLUDE_DIRS}")
endif ()

