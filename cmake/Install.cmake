list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
message("Cmake Binary Dir: ${CMAKE_BINARY_DIR}")

#if (EMSCRIPTEN)
#    install(
#        TARGETS ${PROJECT_NAME}
#        RUNTIME DESTINATION "bin"
#        LIBRARY DESTINATION "lib"
#        ARCHIVE DESTINATION "lib"
#    )
#    install(
#            TARGETS ${PROJECT_NAME}Bin
#            DESTINATION "."
#    )
#else ()
#
#endif ()

file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")
install(
    FILES ${HEADERS}
    DESTINATION "include"
    COMPONENT dev
)

#if (NOT EMSCRIPTEN)
#    install(
#            TARGETS ${LUA_SHARED_LIB}
#            COMPONENT runtime
#            RUNTIME DESTINATION "bin"
#            LIBRARY DESTINATION "lib"
#            ARCHIVE DESTINATION "lib"
#    )
#endif ()
#install(
#        TARGETS ${LUA_SHARED_LIB}
#        COMPONENT dev
#        RUNTIME DESTINATION "lib"
#        LIBRARY DESTINATION "lib"
#        ARCHIVE DESTINATION "lib"
#)

if (WIN32)
    # install(FILES "${CMAKE_BINARY_DIR}//${PROJECT_NAME}.dll" DESTINATION "lib")
    message("-- Installing for Windows")
    install(
            TARGETS ${PROJECT_NAME}Shared ${PROJECT_NAME}Bin ${LUA_SHARED_LIB}
            RUNTIME DESTINATION "."
            COMPONENT runtime
    )
    install(
            TARGETS ${PROJECT_NAME}Shared ${LUA_SHARED_LIB}
            COMPONENT dev
            RUNTIME DESTINATION "lib"
            LIBRARY DESTINATION "lib"
            ARCHIVE DESTINATION "lib"
    )
    install(
        FILES $<TARGET_FILE:${PROJECT_NAME}Shared>
        DESTINATION "lib"
        COMPONENT dev
    )
    install(
        FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}Bin>
        DESTINATION "."
        COMPONENT runtime
    )
    # message("TESTEEE $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}Bin>")
elseif (EMSCRIPTEN)
    message("-- Installing for Emscripten")
    install(
        TARGETS ${PROJECT_NAME}Static ${LUA_SHARED_LIB}
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        COMPONENT dev
    )

    install(
        FILES $<TARGET_FILE:${PROJECT_NAME}Bin>
        DESTINATION "."
        RENAME "index.html"
        COMPONENT runtime
    )
    install(
        FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}Bin>/${PROJECT_NAME}.js" "$<TARGET_FILE_DIR:${PROJECT_NAME}Bin>/${PROJECT_NAME}.wasm"
        DESTINATION "."
        COMPONENT runtime
    )
    # set(CPACK_COMPONENTS_ALL runtime)
elseif (APPLE)
    message("-- Installing for Apple/macOS")
    install(
        TARGETS ${PROJECT_NAME}Shared ${PROJECT_NAME}Bin ${LUA_SHARED_LIB}
        COMPONENT runtime
        RUNTIME DESTINATION "MacOS"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        BUNDLE DESTINATION "."
    )
    install(
        TARGETS ${PROJECT_NAME}Shared ${LUA_SHARED_LIB}
        COMPONENT dev
        RUNTIME DESTINATION "lib"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
    )
    # macOS specific resources
    install(
        FILES "${CMAKE_SOURCE_DIR}/misc/cross/macos/${PROJECT_NAME}_icon.icns"
        DESTINATION "Resources"
        COMPONENT runtime
        RENAME "${PROJECT_NAME}.icns"
    )
    # Create Info.plist for the application bundle
    configure_file(
        "${CMAKE_SOURCE_DIR}/misc/cross/macos/Info.plist.in"
        "${CMAKE_BINARY_DIR}/Info.plist"
    )
    install(
        FILES "${CMAKE_BINARY_DIR}/Info.plist"
        DESTINATION "."
        COMPONENT runtime
    )
elseif (UNIX)
    message("-- Installing for Unix")
    install(
        TARGETS ${PROJECT_NAME}Shared ${PROJECT_NAME}Bin ${LUA_SHARED_LIB}
        COMPONENT runtime
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
    )
    install(
        TARGETS ${PROJECT_NAME}Shared ${PROJECT_NAME}Static ${LUA_SHARED_LIB}
        COMPONENT dev
        RUNTIME DESTINATION "lib"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
    )
    if (EXPORT_APPIMAGE)
        install(FILES "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}_icon.png" DESTINATION "." COMPONENT runtime)
        # file(WRITE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop""")
        install(FILES "${CMAKE_SOURCE_DIR}/misc/cross/AppImage/${PROJECT_NAME}.desktop" DESTINATION "." COMPONENT runtime)
    else ()
        install(FILES "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}_icon.png" DESTINATION "share/icons" COMPONENT runtime)
        # file(WRITE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop""")
        install(FILES "${CMAKE_SOURCE_DIR}/misc/cross/AppImage/${PROJECT_NAME}.desktop" DESTINATION "share/applications" COMPONENT runtime)
    endif ()
endif ()

include(CPackConfig)

cpack_add_component(dev
    DISPLAY_NAME "Development library"
    DESCRIPTION "Install library for development"
)

cpack_add_component(runtime
    REQUIRED
    DISPLAY_NAME "Runtime binaries"
    DESCRIPTION "Install binary for runtime"
)

if (APPLE)
set_target_properties(${PROJECT_NAME}Bin PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${MACOSX_BUNDLE_INFO_PLIST}
    MACOSX_BUNDLE_BUNDLE_NAME ${CPACK_BUNDLE_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${SELENE_VERSION}
)
endif()