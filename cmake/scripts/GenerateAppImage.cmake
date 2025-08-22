include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)
message("Generating AppImage for [selene v${SELENE_VERSION}]")

file(WRITE "${CPACK_TEMPORARY_DIRECTORY}/runtime/AppRun" [[#!/bin/bash
export LD_LIBRARY_PATH="${APPDIR}/lib:${LD_LIBRARY_PATH}"
exec $APPDIR/bin/selene "$@"]])
file(CHMOD "${CPACK_TEMPORARY_DIRECTORY}/runtime/AppRun" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

if(NOT TARGET_ARCHITECTURE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(TARGET_ARCHITECTURE "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(TARGET_ARCHITECTURE "aarch64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
        set(TARGET_ARCHITECTURE "i686")
    else()
        message(FATAL_ERROR "No supported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

message("Searching for appimagetool in ${CMAKE_CURRENT_LIST_DIR}/../../.cache")
find_program(APPIMAGETOOL appimagetool-${TARGET_ARCHITECTURE} PATHS "${CMAKE_CURRENT_LIST_DIR}/../../.cache" REQUIRED)

#message("Package directory: ${CPACK_TEMPORARY_DIRECTORY} ${COMPONENT}")
execute_process(
    COMMAND env ARCH=${TARGET_ARCHITECTURE}
        ${APPIMAGETOOL}
        "${CPACK_TEMPORARY_DIRECTORY}/runtime"
        "${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}_${TARGET_ARCHITECTURE}.AppImage"
    RESULT_VARIABLE appimage_result
    COMMAND_ECHO STDOUT
)

if(NOT appimage_result EQUAL 0)
    message(FATAL_ERROR "Failed to generate AppImage (code: ${appimage_result})")
else()
    message(STATUS "AppImage generated with success in: ${APPIMAGE_OUTPUT_DIR}/${APPIMAGE_NAME}")
endif()