# window module: window creation module plus the SDL2/GLFW backends.
file(GLOB WINDOW_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/*.c)
selene_add_module_sources(${WINDOW_SOURCES})
if (MSVC)
    source_group("Source Files\\window" FILES ${WINDOW_SOURCES})
endif ()
