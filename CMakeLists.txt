# ---------------------------------------------------------------------------
# selene — top level.  Only project setup and subprojects live here:
#   * cmake/Config.cmake         version + options + cache variables
#   * cmake/CheckPlatform.cmake  platform/triple detection
#   * cmake/SeleneSettings.cmake global flags and output directories
#   * third/                     bundled dependencies (cglm, miniz, glad)
#   * src/                       runtime library, modules and front-ends
# ---------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

include(Config)

project(${SELENE_NAME} VERSION ${SELENE_VERSION} LANGUAGES C CXX)

# Single-config generators need a build type (otherwise CMAKE_BUILD_TYPE is
# empty and the output directories become build//bin).  Multi-config
# generators (Visual Studio) are left alone.
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif ()

if (MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif ()

# Platform / architecture / triple detection.
include(CheckPlatform)
include(CheckTriple)

if (ANDROID)
    set(ANDROID_HOME "$ENV{ANDROID_HOME}" CACHE STRING "Set Android SDK home path")
    message("Android ABI: ${ANDROID_ABI}")
    message("Android Platform: ${ANDROID_PLATFORM}")
endif ()

if (WIN32)
    ENABLE_LANGUAGE(RC)
endif ()

# Generated configuration header (SELENE_VERSION, module metanames, ...).
configure_file("${CMAKE_SOURCE_DIR}/include/selene_config.h.in"
               "${CMAKE_SOURCE_DIR}/include/selene_config.h")

# Global build settings shared by every target.
include(SeleneSettings)

# --- Subprojects -----------------------------------------------------------
add_subdirectory(third)
add_subdirectory(src)

# --- Packaging / tests -----------------------------------------------------
include(Install)

enable_testing()
add_test(NAME ${PROJECT_NAME}Test
         COMMAND $<TARGET_FILE:${PROJECT_NAME}Bin> -d ${CMAKE_SOURCE_DIR}/examples/test)
