# include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)

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

set(ARCHS "x86_64" "aarch64")

foreach(ARCH IN LISTS ARCHS)
    if (NOT EXISTS "/usr/bin/appimagetool-${ARCH}")
        file(DOWNLOAD "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${ARCH}.AppImage" "/usr/bin/appimagetool-${ARCH}" SHOW_PROGRESS)
        file(CHMOD "/usr/bin/appimagetool-${ARCH}" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
    endif ()
endforeach()


# Baixa o AppImageTool se não existir
# set(APPIMAGE_TOOL_URL "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${TARGET_ARCHITECTURE}.AppImage")
# set(APPIMAGE_TOOL_DEST "/usr/bin/appimagetool-${TARGET_ARCHITECTURE}")

# if (NOT EXISTS ${APPIMAGE_TOOL_DEST})
#     file(DOWNLOAD "${APPIMAGE_TOOL_URL}" "${APPIMAGE_TOOL_DEST}" SHOW_PROGRESS)
#     file(CHMOD "${APPIMAGE_TOOL_DEST}" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
# endif ()
# set(APPIMAGE_TOOL_PATH "${APPIMAGE_TOOL_DEST}" CACHE FILEPATH "AppImageTool path")