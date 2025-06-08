include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)

string(REPLACE ";" "','" ANDROID_ABI_FILTERS_STRING "${ANDROID_ABIS}")
set(ANDROID_ABI_FILTERS "'${ANDROID_ABI_FILTERS_STRING}'")

set(CMAKE_MAIN_PROJECT_PATH "${CMAKE_CURRENT_LIST_DIR}/../../CMakeLists.txt")

file(
    COPY
    "${CMAKE_SOURCE_DIR}/misc/cross/android"
    DESTINATION "${CMAKE_SOURCE_DIR}/build/"
)

configure_file(
        "${CMAKE_SOURCE_DIR}/misc/cross/android/app/build.gradle.in"
        "${CMAKE_SOURCE_DIR}/build/android/app/build.gradle"
        @ONLY
)

if (WIN32)
    set(GRADLE_FILENAME gradlew.bat)
else ()
    set(GRADLE_FILENAME gradlew)
endif ()

find_program(GRADLE_EXECUTABLE
        NAMES ${GRADLE_FILENAME}
        PATHS "${CMAKE_SOURCE_DIR}/build/android"
        REQUIRED
)

execute_process(
        COMMAND ${GRADLE_EXECUTABLE} assembleDebug
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build/android"
        RESULT_VARIABLE gradle_result
)

if(NOT gradle_result EQUAL 0)
    message(FATAL_ERROR "Failed to execute gradle!")
endif()

execute_process(
        COMMAND ${GRADLE_EXECUTABLE} assemble
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build/android"
        RESULT_VARIABLE gradle_result
)

if(NOT gradle_result EQUAL 0)
    message(FATAL_ERROR "Failed to execute gradle!")
endif()