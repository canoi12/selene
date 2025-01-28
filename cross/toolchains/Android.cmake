if ("$ENV{NDK_HOME}" STREQUAL "")
    message(FATAL_ERROR "NDK_HOME must be set")
endif ()
file(REAL_PATH "$ENV{NDK_HOME}/build/cmake/android.toolchain.cmake" ANDROID_TOOLCHAIN_FILE BASE_DIRECTORY "${CMAKE_SOURCE_DIR}")

include("${ANDROID_TOOLCHAIN_FILE}")

message("Android: ${ANDROID} ${ANDROID_TOOLCHAIN_FILE}")
if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7-a")
set(triple "armv7-linux-android")
elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
set(triple "x86_64-linux-android")
endif()