# ---------------------------------------------------------------------------
# Global build settings shared by every target: build-type defines, output
# directories, linker/compiler flags, the platform library suffix and LuaJIT.
# Included from the top-level CMakeLists.txt.
# ---------------------------------------------------------------------------

# Debug/NDEBUG (MSVC drives this through its own per-configuration flags).
if (NOT MSVC)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-DDEBUG)
    else ()
        add_definitions(-DNDEBUG)
    endif ()
endif ()

# Output layout.  Single-config generators (Ninja/Make) put the build type in
# the path; multi-config generators (Visual Studio) do not.
if (NOT DEFINED MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
    if (NOT NINTENDO_SWITCH)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm")
    endif ()
else ()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif ()

if (DEFINED EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sWASM=1")
endif ()

# Library suffix used to locate prebuilt artifacts (lua, ...) per platform.
if (NOT DEFINED LIB_PLATFORM)
    if (ANDROID)
        set(LIB_PLATFORM "/${ANDROID_PLATFORM}")
    elseif (DEFINED MSVC)
        set(LIB_PLATFORM "/v143")
    else ()
        set(LIB_PLATFORM "")
    endif ()
endif ()

if (DEFINED MINGW)
    add_definitions(-DMINGW)
    if (SELENE_NO_D3D11)
        add_definitions(-DSELENE_NO_D3D11)
    endif ()
endif ()

if ("${LUA_VERSION}" STREQUAL "jit")
    message("Using Lua JIT")
    add_definitions(-DSELENE_USE_JIT)
endif ()
