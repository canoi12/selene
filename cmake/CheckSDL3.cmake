message("Check For SDL3 lib")

if (DEFINED MSVC)
    set (SDL_PRECOMP_DIR "${CACHE_DIR}/SDL3/MSVC" CACHE STRING "Set SDL3 dir")
elseif (DEFINED MINGW)
    set (SDL_PRECOMP_DIR "${CACHE_DIR}/SDL3/MINGW" CACHE STRING "Set SDL3 dir")
else ()
    set (SDL_PRECOMP_DIR "" CACHE STRING "Set SDL3 dir")
endif ()
set (SDL_SOURCE_DIR "${CACHE_DIR}/SDL3/source" CACHE STRING "Set SDL3 source dir")

if (NOT "${SDL_PRECOMP_DIR}" STREQUAL "")
set(SDL3_DIR "${SDL_PRECOMP_DIR}/cmake")
endif ()
message("testing builds/${triple}${LIB_PLATFORM}")
if (DEFINED SDL3_DIR)
    find_package(SDL3 REQUIRED)
else ()
    find_library(SDL3_Lib NAMES SDL3)
    if (NOT SDL3_Lib)
        message ("SDL3 Lib not found in system, looking for precompiled in ${CMAKE_SOURCE_DIR}/builds/${triple}${LIB_PLATFORM}")
        find_library(SDL3_Lib NAMES SDL3 PATHS "${CMAKE_SOURCE_DIR}/builds/${triple}${LIB_PLATFORM}/lib" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    endif ()
    # find_library(SDL3_Main_Lib NAMES SDL3_main PATHS "${CMAKE_SOURCE_DIR}/builds/${triple}${LIB_PLATFORM}")
    find_path(SDL3_INCLUDE_DIR NAMES SDL3/SDL.h)
    if (NOT SDL3_INCLUDE_DIR)
        message ("Not found")
        find_path(SDL3_INCLUDE_DIR NAMES SDL3/SDL.h PATHS "${SDL_SOURCE_DIR}/include" REQUIRED NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        set (SDL3_INCLUDE_DIRS ${SDL3_INCLUDE_DIR})
    else ()
        set (SDL3_INCLUDE_DIRS ${SDL3_INCLUDE_DIR}/SDL3)
    endif ()
endif ()

message("SDL3 Library ${SDL3_Lib}")
message("SDL3 INCLUDE ${SDL3_INCLUDE_DIRS}")

if ((NOT SDL3_FOUND AND SDL3_Lib) AND NOT TARGET SDL3::SDL3-shared)
    message ("Import SDL3 Library")
    add_library(SDL3::SDL3-shared SHARED IMPORTED)
    set_property(TARGET SDL3::SDL3-shared PROPERTY IMPORTED_LOCATION ${SDL3_Lib})
    set (SDL3_LIBRARIES SDL3::SDL3-shared)
elseif (NOT SDL3_Lib AND NOT SDL3_FOUND)
    message("Compiling SDL3 from source")
    add_subdirectory(${SDL_SOURCE_DIR})
    set (SDL3_LIBRARIES SDL3-shared)
endif ()

