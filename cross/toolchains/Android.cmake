if ("$ENV{NDK_HOME}" STREQUAL "")
    message(FATAL_ERROR "NDK_HOME must be set")
endif ()
file(REAL_PATH "$ENV{NDK_HOME}/build/cmake/android.toolchain.cmake" ANDROID_TOOLCHAIN_FILE BASE_DIRECTORY "${CMAKE_SOURCE_DIR}")

include("${ANDROID_TOOLCHAIN_FILE}")

if ("${ANDROID_ABI}" STREQUAL "arm64-v8a")
    set(triple "aarch64-linux-android")
elseif ("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
    set(triple "armv7-linux-android")
elseif ("${ANDROID_ABI}" STREQUAL "x86")
    set(triple "i386-linux-android")
elseif ("${ANDROID_ABI}" STREQUAL "x86_64")
    set(triple "x86_64-linux-android")
endif ()

# set(CMAKE_PREFIX_PATH "")
# set(CMAKE_SYSROOT "")