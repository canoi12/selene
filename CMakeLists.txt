cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(selene VERSION 0.1.0 LANGUAGES C CXX)

message("Compiling for ${CMAKE_SYSTEM_NAME}")

set(USE_FLAGS "")
if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s FULL_ES2 -sALLOW_MEMORY_GROWTH -sFORCE_FILESYSTEM --embed-file core/ --emrun")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    set(EMSCRIPTEN 1)
endif()

#include_directories(./ external/)
# include_directories(src/ src/sdl2/ src/gl/ src/image/ src/audio/)
include_directories(src/ src/modules/ src/modules/*/)
include_directories(external/ external/lua/src/ external/glad/include/)

if (MSVC)
    set(SDL2_DIR external/SDL2/MSVC/cmake)
    set(COPY_DLL TRUE)
elseif(MINGW)
    set(SDL2_DIR external/SDL2/MINGW/cmake)
    set(COPY_DLL TRUE)
endif()

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${SDL2_INCLUDE_DIRS})

set(LIBRARIES ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})

if (WIN32)
    if (MINGW)
        set(LIBRARIES mingw32 ${LIBRARIES})
    endif()
    set(LIBRARIES ${LIBRARIES} -mwindows gdi32 winmm)
else()
    set(LIBRARIES ${LIBRARIES} dl m pthread)
endif()

# file(GLOB selene_source selene.c src/*.c external/glad/src/glad.c)
file(GLOB selene_source src/selene.c src/Data.c src/modules/*/*.c external/glad/src/glad.c)
file(GLOB lua_source external/lua/src/*.c)

add_library(lua STATIC ${lua_source})
# target_link_libraries(lua m)

add_executable(${PROJECT_NAME} ${selene_source})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} lua)

if (COPY_DLL)
    if (MSVC)
        file(GLOB SDL_LIB external/SDL2/MSVC/lib/x64/SDL2.dll)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL_LIB} "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    elseif(MINGW)
        file(GLOB SDL_LIB external/SDL2/MINGW/${TOOLCHAIN_PREFIX}/bin/SDL2.dll)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL_LIB} "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif()
endif()

file(GLOB SCRIPTS_DIR scripts/)
add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/scripts $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    # ${CMAKE_SOURCE_DIR}/test $<TARGET_FILE_DIR:${PROJECT_NAME}>/core
)
