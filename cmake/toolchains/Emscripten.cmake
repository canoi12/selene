if (NOT DEFINED ENV{EMSDK_DIR})
    message("Setting EMSDK_DIR: ${CMAKE_SOURCE_DIR}/.cache/emsdk-latest")
    set(ENV{EMSDK_DIR} "${CMAKE_SOURCE_DIR}/.cache/emsdk-latest")
    # set(CMAKE_MODULE_PATH "$ENV{EMSKD_DIR}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake;${CMAKE_MODULE_PATH}")
endif ()
message("Set toolchain $ENV{EMSDK_DIR}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")

set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_C_COMPILER "$ENV{EMSDK_DIR}/emscripten/incoming/emcc")
set(CMAKE_CXX_COMPILER "$ENV{EMSDK_DIR}/emscripten/incoming/em++")

#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

include("$ENV{EMSDK_DIR}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")