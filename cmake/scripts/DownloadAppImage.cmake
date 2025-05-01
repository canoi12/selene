include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)

# Baixa o AppImageTool se não existir
set(APPIMAGE_TOOL_URL "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage")
set(APPIMAGE_TOOL_DEST "${CACHE_DIR}/appimagetool")

if (NOT EXISTS ${APPIMAGE_TOOL_DEST})
    file(DOWNLOAD "${APPIMAGE_TOOL_URL}" "${APPIMAGE_TOOL_DEST}" SHOW_PROGRESS)
    file(CHMOD "${APPIMAGE_TOOL_DEST}" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
endif ()
set(APPIMAGE_TOOL_PATH "${APPIMAGE_TOOL_DEST}" CACHE FILEPATH "AppImageTool path")