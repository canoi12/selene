# ---------------------------------------------------------------------------
# SDL selection.  Picks SDL2 or SDL3 (SELENE_USE_SDL3) and exports:
#   SELENE_SDL_INCLUDE_DIRS, SELENE_SDL_LIBRARIES
# On Emscripten SDL2 comes from the Emscripten port (-sUSE_SDL=2) and no
# package lookup happens.  Included from src/CMakeLists.txt.
# ---------------------------------------------------------------------------
if (DEFINED EMSCRIPTEN)
    set(SELENE_USE_SDL3 OFF)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_SDL=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_SDL=2")
    set(SELENE_SDL_INCLUDE_DIRS "")
    set(SELENE_SDL_LIBRARIES "")
elseif (DEFINED NINTENDO_SWITCH)
    set(SELENE_USE_SDL3 OFF)
    add_library(SDL2::SDL2 STATIC IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL_DIR}/lib/Release/SDL.a")
    set(SELENE_SDL_LIBRARIES SDL2::SDL2)
    set(SELENE_SDL_INCLUDE_DIRS "${SDL_DIR}/include")
elseif (SELENE_USE_SDL3)
    find_package(CustomSDL3 REQUIRED)
    add_definitions(-DSELENE_USE_SDL3)
    set(SELENE_SDL_INCLUDE_DIRS "${SDL3_INCLUDE_DIRS}")
    set(SELENE_SDL_LIBRARIES "${SDL3_LIBRARIES}")
else ()
    find_package(CustomSDL2 REQUIRED)
    set(SELENE_SDL_INCLUDE_DIRS "${SDL2_INCLUDE_DIRS}")
    set(SELENE_SDL_LIBRARIES "${SDL2_LIBRARIES}")
endif ()
