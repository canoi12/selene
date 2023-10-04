cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(selene VERSION 0.1.0 LANGUAGES C CXX)

message("Compiling for ${CMAKE_SYSTEM_NAME}")

set(USE_FLAGS "")
if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(USE_FLAGS "${USE_FLAGS} -s EXPORTED_FUNCTIONS=\"['_malloc']\" --embed-file core/")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_SDL=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    set(EMSCRIPTEN 1)
endif()

include_directories(./ external/)
include_directories(external/lua/src/ external/glad/include/)

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

file(GLOB selene_source selene.c src/*.c external/glad/src/glad.c)
file(GLOB lua_source external/lua/src/*.c)

add_library(lua STATIC ${lua_source})
target_link_libraries(lua m)

add_executable(${PROJECT_NAME} ${selene_source})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} lua)

if (EMSCRIPTEN MATCHES 1)
    message("Entra aqui??")
    file(GLOB CORE_DIR core/)
    add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/core $<TARGET_FILE_DIR:${PROJECT_NAME}>/core
    )
endif()
