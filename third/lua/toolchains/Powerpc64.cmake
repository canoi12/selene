set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
set(CMAKE_CROSSCOMPILING true)

set(triple "powerpc64-linux-gnu")

set(CMAKE_FIND_ROOT_PATH "/usr/${triple}")
# set(CMAKE_SYSROOT "/usr/${triple}")

set(CMAKE_C_COMPILER ${triple}-gcc)
set(CMAKE_CXX_COMPILER ${triple}-g++)

set(CMAKE_INCLUDE_PATH "/usr/${triple}/include")
set(CMAKE_LIBRARY_PATH "/usr/${triple}/lib")
set(CMAKE_PROGRAM_PATH "/usr/${triple}/bin")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_COMPILER_WORKS true)
message("Building for Aarch64")
message("Lib: ${CMAKE_LIBRARY_PATH}")