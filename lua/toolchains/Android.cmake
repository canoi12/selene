if ("$ENV{NDK_HOME}" STREQUAL "")
    message(FATAL_ERROR "NDK_HOME must be set")
endif ()
include("$ENV{NDK_HOME}/build/cmake/android.toolchain.cmake")