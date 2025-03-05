message("Check for Lua lib")

set (LUA_DIST_DIR "${CMAKE_SOURCE_DIR}/builds/${triple}${LIB_PLATFORM}")
set (LUA_PROJECT_DIR "${CMAKE_SOURCE_DIR}/third/lua/lua${LUA_VERSION}")
set (LUA_TARGET_NAME "lua${LUA_VERSION}")

find_library(LUA_LIBRARY NAMES ${LUA_TARGET_NAME} PATHS "${LUA_DIST_DIR}/lib" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
find_path(LUA_INCLUDE_DIR NAMES lua.h PATHS "${LUA_PROJECT_DIR}/include" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

message("LUA LIBRARY: ${LUA_LIBRARY}")
message("LUA INCLUDE DIR: ${LUA_INCLUDE_DIR}")

set(LUA_LIB_TYPE "SHARED")
if ((DEFINED WIN32) OR (DEFINED EMSCRIPTEN))
    set(LUA_LIB_TYPE "STATIC")
endif ()

if (NOT LUA_LIBRARY)
    message("Compiling Lua from source")
    add_subdirectory(${LUA_PROJECT_DIR})
elseif (LUA_LIBRARY AND NOT ${LUA_TARGET_NAME})
    add_library(${LUA_TARGET_NAME} ${LUA_LIB_TYPE} IMPORTED)
    set_target_properties(
        ${LUA_TARGET_NAME} PROPERTIES
        IMPORTED_LOCATION ${LUA_LIBRARY}
    )
endif ()

if (LUA_INCLUDE_DIR)
    include_directories(${LUA_INCLUDE_DIR})
else ()
    include_directories("${LUA_PROJECT_DIR}/include")
endif ()