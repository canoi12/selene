#[=======================================================================[.rst:
FindCustomSDL2
--------------

Locates SDL2 for selene.

Search order:

1. ``SDL2_DIR`` (directory containing ``SDL2Config.cmake`` /
   ``sdl2-config.cmake``).  When it is not set, a config package found inside
   ``SDL_DIR`` is used.
2. Manual lookup using ``SDL_DIR`` as the only hint (``NO_DEFAULT_PATH``) when
   ``SDL_DIR`` was explicitly provided.
3. Regular ``find_package(SDL2 CONFIG)`` (system / ``CMAKE_PREFIX_PATH``).
4. Manual lookup using ``SDL_DIR`` as hint plus the default search paths.
5. Build SDL2 from source when ``SDL_DIR`` (or the legacy ``SDL_SOURCE_DIR``)
   holds an SDL2 source tree, or when ``USE_SDL_SOURCE`` is ON.

Input variables
^^^^^^^^^^^^^^^

``SDL_DIR``
  Root of an SDL2 installation, of a precompiled package (MSVC/MinGW devel
  archives) or of the SDL2 source tree.  When unset it defaults to
  ``${CACHE_DIR}/SDL2-${SDL_VERSION}/{MSVC,MINGW,source}`` depending on the
  toolchain.  Keep this default in sync with ``CMakePresets.json``.

``SDL_PRECOMP_DIR``
  Deprecated alias of ``SDL_DIR``.

``SDL2_DIR``
  Standard CMake package directory; takes precedence over ``SDL_DIR``.

``SDL2_USE_STATIC``
  Prefer ``SDL2::SDL2-static`` when the package provides it.

``USE_SDL_SOURCE``
  Force building SDL2 from source (requires an SDL2 source tree).

Result variables
^^^^^^^^^^^^^^^^

``SDL2_FOUND``
``SDL2_INCLUDE_DIRS``
``SDL2_LIBRARIES``
``SDL2_DLL`` (Windows only)

Imported targets
^^^^^^^^^^^^^^^^

``SDL2::SDL2``
``SDL2::SDL2main`` (Windows only)
#]=======================================================================]

include_guard(GLOBAL)

# --- Default SDL_DIR (platform aware; keep in sync with CMakePresets.json) ---
if (MSVC)
    set(_sdl2_default_dir "${CACHE_DIR}/SDL2-${SDL_VERSION}/MSVC")
elseif (MINGW)
    set(_sdl2_default_dir "${CACHE_DIR}/SDL2-${SDL_VERSION}/MINGW")
else ()
    set(_sdl2_default_dir "${CACHE_DIR}/SDL2-${SDL_VERSION}/source")
endif ()

if (NOT SDL_DIR)
    set(SDL_DIR "${_sdl2_default_dir}" CACHE PATH
        "Root of an SDL2 installation, precompiled package or source tree")
endif ()

file(TO_CMAKE_PATH "${SDL_DIR}" _sdl2_root)
string(REGEX REPLACE "/+$" "" _sdl2_root "${_sdl2_root}")

file(TO_CMAKE_PATH "${_sdl2_default_dir}" _sdl2_default_root)
string(REGEX REPLACE "/+$" "" _sdl2_default_root "${_sdl2_default_root}")

set(_sdl2_explicit FALSE)
if (NOT "${_sdl2_root}" STREQUAL "" AND
    NOT "${_sdl2_root}" STREQUAL "${_sdl2_default_root}")
    set(_sdl2_explicit TRUE)
endif ()

# --- Layouts ---------------------------------------------------------------
# MinGW/Android archives nest headers, libraries and cmake files inside a
# target-triple directory.
set(_sdl2_vendor_roots "")
file(GLOB _sdl2_vendor_roots
    "${_sdl2_root}/*-w64-mingw32"
    "${_sdl2_root}/*-linux-android")
if (DEFINED triple AND NOT "${triple}" STREQUAL "" AND
    EXISTS "${_sdl2_root}/${triple}")
    list(APPEND _sdl2_vendor_roots "${_sdl2_root}/${triple}")
endif ()
list(REMOVE_DUPLICATES _sdl2_vendor_roots)

set(_sdl2_search_roots "${_sdl2_root}")
list(APPEND _sdl2_search_roots ${_sdl2_vendor_roots})

# --- Source tree detection -------------------------------------------------
set(_sdl2_source_tree "")
foreach (_sdl2_candidate IN LISTS _sdl2_search_roots)
    if (EXISTS "${_sdl2_candidate}/CMakeLists.txt" AND
        EXISTS "${_sdl2_candidate}/include/SDL.h")
        set(_sdl2_source_tree "${_sdl2_candidate}")
        break()
    endif ()
endforeach ()

# Legacy fallback kept for backwards compatibility (cmake/Config.cmake).
if ("${_sdl2_source_tree}" STREQUAL "" AND SDL_SOURCE_DIR AND
    EXISTS "${SDL_SOURCE_DIR}/CMakeLists.txt" AND
    EXISTS "${SDL_SOURCE_DIR}/include/SDL.h")
    set(_sdl2_source_tree "${SDL_SOURCE_DIR}")
endif ()

# --- Invalidate cached lookups when SDL_DIR changes ------------------------
if (NOT DEFINED SELENE_SDL2_DIR_HINT)
    set(SELENE_SDL2_DIR_HINT "" CACHE INTERNAL
        "SDL_DIR value used by FindCustomSDL2")
endif ()

if (NOT "${SELENE_SDL2_DIR_HINT}" STREQUAL "${_sdl2_root}")
    if (SELENE_SDL2_DIR_AUTODETECTED)
        unset(SDL2_DIR CACHE)
    endif ()
    unset(SDL2_INCLUDE_DIR CACHE)
    unset(SDL2_LIBRARY CACHE)
    unset(SDL2main_LIBRARY CACHE)
    unset(SDL2_DLL CACHE)
    set(SELENE_SDL2_DIR_HINT "${_sdl2_root}" CACHE INTERNAL
        "SDL_DIR value used by FindCustomSDL2")
    set(SELENE_SDL2_DIR_AUTODETECTED FALSE CACHE INTERNAL
        "Whether SDL2_DIR was detected by FindCustomSDL2")
endif ()

# --- Manual discovery ------------------------------------------------------
# Creates SDL2::SDL2 (and SDL2::SDL2main on Windows) from
# SDL2_INCLUDE_DIR / SDL2_LIBRARY / SDL2main_LIBRARY / SDL2_DLL.
function(_selene_sdl2_discover no_default_path)
    set(_extra_args "")
    if (no_default_path)
        list(APPEND _extra_args NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    endif ()

    find_path(SDL2_INCLUDE_DIR
        NAMES SDL.h SDL2/SDL.h
        HINTS ${SDL2_SEARCH_HINTS}
        ${_extra_args})
    find_library(SDL2_LIBRARY
        NAMES ${SDL2_LIBRARY_NAMES}
        HINTS ${SDL2_SEARCH_HINTS}
        ${_extra_args})

    if (WIN32)
        find_library(SDL2main_LIBRARY
            NAMES SDL2main
            HINTS ${SDL2_SEARCH_HINTS}
            ${_extra_args})
        find_file(SDL2_DLL
            NAMES SDL2.dll
            HINTS ${SDL2_SEARCH_HINTS}
            ${_extra_args})
    endif ()

    if (NOT SDL2_INCLUDE_DIR OR NOT SDL2_LIBRARY)
        return()
    endif ()

    if (EXISTS "${SDL2_INCLUDE_DIR}/SDL2/SDL.h")
        set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}/SDL2")
    else ()
        set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}")
    endif ()
    set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIRS}" PARENT_SCOPE)

    if (TARGET SDL2::SDL2)
        set(SDL2_TARGET_FOUND TRUE PARENT_SCOPE)
        return()
    endif ()

    # Shared vs static.  ``SDL2_USE_STATIC`` always wins; otherwise the
    # library file name decides.
    set(_shared FALSE)
    get_filename_component(_lib_name "${SDL2_LIBRARY}" NAME)
    if (SDL2_USE_STATIC)
        set(_shared FALSE)
    elseif (_lib_name MATCHES "\\.dll\\.a$" OR
            _lib_name MATCHES "\\.so(\\.[0-9.]+)?$" OR
            _lib_name MATCHES "\\.dylib$")
        set(_shared TRUE)
    elseif (WIN32 AND _lib_name MATCHES "\\.lib$")
        set(_shared TRUE)
    endif ()

    if (_shared)
        add_library(SDL2::SDL2 SHARED IMPORTED)
        if (SDL2_DLL)
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION "${SDL2_DLL}"
                IMPORTED_IMPLIB "${SDL2_LIBRARY}")
        else ()
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION "${SDL2_LIBRARY}")
        endif ()
    else ()
        add_library(SDL2::SDL2 STATIC IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION "${SDL2_LIBRARY}")
    endif ()
    set_target_properties(SDL2::SDL2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}")

    if (WIN32 AND SDL2main_LIBRARY AND NOT TARGET SDL2::SDL2main)
        add_library(SDL2::SDL2main STATIC IMPORTED)
        set_target_properties(SDL2::SDL2main PROPERTIES
            IMPORTED_LOCATION "${SDL2main_LIBRARY}"
            INTERFACE_LINK_LIBRARIES SDL2::SDL2)
    endif ()

    set(SDL2_TARGET_FOUND TRUE PARENT_SCOPE)
endfunction()

set(SDL2_LIBRARY_NAMES SDL2 SDL2-static)
if (SDL2_USE_STATIC)
    set(SDL2_LIBRARY_NAMES SDL2-static SDL2)
endif ()

set(SDL2_SEARCH_HINTS "")
foreach (_sdl2_dir IN LISTS _sdl2_search_roots)
    # Do not mix headers from an SDL2 source tree with system libraries.
    if (NOT "${_sdl2_dir}" STREQUAL "${_sdl2_source_tree}")
        list(APPEND SDL2_SEARCH_HINTS
            "${_sdl2_dir}/include/SDL2"
            "${_sdl2_dir}/include"
            "${_sdl2_dir}"
            "${_sdl2_dir}/lib"
            "${_sdl2_dir}/lib64"
            "${_sdl2_dir}/lib/x64"
            "${_sdl2_dir}/lib/x86"
            "${_sdl2_dir}/bin")
    endif ()
endforeach ()


# --- Resolution ------------------------------------------------------------
set(_sdl2_target "")

set(_sdl2_pkg_dir "")
if (SDL2_DIR)
    set(_sdl2_pkg_dir "${SDL2_DIR}")
endif ()

if (USE_SDL_SOURCE)
    if ("${_sdl2_source_tree}" STREQUAL "")
        message(FATAL_ERROR
            "USE_SDL_SOURCE is ON but no SDL2 source tree was found.\n"
            "Point SDL_DIR (or SDL_SOURCE_DIR) to an SDL2 source checkout, "
            "e.g. -DSDL_DIR=${CACHE_DIR}/SDL2-${SDL_VERSION}/source")
    endif ()
else ()
    # 1. Config package inside SDL_DIR.
    if (EXISTS "${_sdl2_root}")
        set(_sdl2_config_paths "")
        foreach (_sdl2_dir IN LISTS _sdl2_search_roots)
            list(APPEND _sdl2_config_paths
                "${_sdl2_dir}"
                "${_sdl2_dir}/cmake"
                "${_sdl2_dir}/lib/cmake/SDL2"
                "${_sdl2_dir}/lib/${triple}/cmake/SDL2"
                "${_sdl2_dir}/share/cmake/SDL2")
        endforeach ()

        if ("${_sdl2_pkg_dir}" STREQUAL "")
            foreach (_sdl2_dir IN LISTS _sdl2_config_paths)
                if (EXISTS "${_sdl2_dir}/SDL2Config.cmake" OR
                    EXISTS "${_sdl2_dir}/sdl2-config.cmake")
                    set(SDL2_DIR "${_sdl2_dir}" CACHE PATH
                        "Directory containing SDL2Config.cmake")
                    set(_sdl2_pkg_dir "${_sdl2_dir}")
                    set(SELENE_SDL2_DIR_AUTODETECTED TRUE CACHE INTERNAL
                        "Whether SDL2_DIR was detected by FindCustomSDL2")
                    break()
                endif ()
            endforeach ()
        endif ()

        if (NOT "${_sdl2_pkg_dir}" STREQUAL "")
            find_package(SDL2 CONFIG QUIET)
        endif ()
    endif ()

    if (TARGET SDL2::SDL2)
        set(_sdl2_target SDL2::SDL2)
    elseif (TARGET SDL2::SDL2-static)
        set(_sdl2_target SDL2::SDL2-static)
    endif ()

    # 2. Explicit SDL_DIR without a cmake package: trust it over the system.
    if (NOT _sdl2_target AND _sdl2_explicit)
        _selene_sdl2_discover(TRUE)
        if (TARGET SDL2::SDL2)
            set(_sdl2_target SDL2::SDL2)
        elseif (TARGET SDL2::SDL2-static)
            set(_sdl2_target SDL2::SDL2-static)
        endif ()
    endif ()

    # 3. Regular config package (system, CMAKE_PREFIX_PATH, SDL2_ROOT, ...).
    if (NOT _sdl2_target)
        find_package(SDL2 CONFIG QUIET)
        if (TARGET SDL2::SDL2)
            set(_sdl2_target SDL2::SDL2)
        elseif (TARGET SDL2::SDL2-static)
            set(_sdl2_target SDL2::SDL2-static)
        endif ()
    endif ()

    # 4. System libraries without a cmake package.
    if (NOT _sdl2_target)
        _selene_sdl2_discover(FALSE)
        if (TARGET SDL2::SDL2)
            set(_sdl2_target SDL2::SDL2)
        elseif (TARGET SDL2::SDL2-static)
            set(_sdl2_target SDL2::SDL2-static)
        endif ()
    endif ()
endif ()

# 5. Build from source.
if (NOT _sdl2_target AND NOT "${_sdl2_source_tree}" STREQUAL "")
    message(STATUS "SDL2: building from source: ${_sdl2_source_tree}")
    add_subdirectory("${_sdl2_source_tree}" "${CMAKE_BINARY_DIR}/_deps/sdl2")

    if (TARGET SDL2 AND NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 ALIAS SDL2)
    elseif (TARGET SDL2-static AND NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 ALIAS SDL2-static)
    endif ()
    if (WIN32 AND TARGET SDL2main AND NOT TARGET SDL2::SDL2main)
        add_library(SDL2::SDL2main ALIAS SDL2main)
    endif ()

    set(SDL2_INCLUDE_DIRS "${_sdl2_source_tree}/include")

    if (TARGET SDL2::SDL2)
        set(_sdl2_target SDL2::SDL2)
    elseif (TARGET SDL2::SDL2-static)
        set(_sdl2_target SDL2::SDL2-static)
    endif ()
endif ()

if (NOT _sdl2_target)
    message(FATAL_ERROR
        "SDL2 not found.\n"
        "  SDL_DIR  = ${SDL_DIR}\n"
        "  SDL2_DIR = ${_sdl2_pkg_dir}\n"
        "Hints:\n"
        "  * -DSDL_DIR=<path>  (precompiled package, installation prefix or source tree)\n"
        "  * -DSDL2_DIR=<path> (directory containing SDL2Config.cmake)\n"
        "  * -DUSE_SDL_SOURCE=ON with a valid SDL2 source tree\n"
        "  * install the SDL2 development files for your platform")
endif ()

# --- Selection / outputs ---------------------------------------------------
if (SDL2_USE_STATIC AND TARGET SDL2::SDL2-static)
    set(_sdl2_target SDL2::SDL2-static)
elseif (TARGET SDL2::SDL2)
    set(_sdl2_target SDL2::SDL2)
elseif (TARGET SDL2::SDL2-static)
    set(_sdl2_target SDL2::SDL2-static)
endif ()

set(SDL2_FOUND TRUE)
set(CustomSDL2_FOUND TRUE)

set(SDL2_LIBRARIES "${_sdl2_target}")
if (WIN32 AND TARGET SDL2::SDL2main)
    list(INSERT SDL2_LIBRARIES 0 SDL2::SDL2main)
endif ()

# Best effort DLL location for packaging (only meaningful on Windows).
if (WIN32 AND NOT SDL2_DLL AND TARGET ${_sdl2_target})
    get_target_property(_sdl2_location ${_sdl2_target} IMPORTED_LOCATION)
    get_target_property(_sdl2_implib ${_sdl2_target} IMPORTED_IMPLIB)
    if (_sdl2_location AND _sdl2_implib)
        set(SDL2_DLL "${_sdl2_location}")
    endif ()
endif ()

message(STATUS "SDL2: using ${_sdl2_target}")
if (SDL2_INCLUDE_DIRS)
    message(STATUS "SDL2: include dirs ${SDL2_INCLUDE_DIRS}")
endif ()

