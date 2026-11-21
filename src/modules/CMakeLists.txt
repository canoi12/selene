# ---------------------------------------------------------------------------
# selene modules.  Each module appends its sources to the selene library
# targets through selene_add_module_sources() (defined in ../CMakeLists.txt).
# ---------------------------------------------------------------------------
add_subdirectory(audio)
add_subdirectory(filesystem)
add_subdirectory(renderer)
add_subdirectory(window)

# --- "Flat" modules (sources directly under src/modules/) ------------------
# font (font.c) and json (ljson.c)
selene_add_module_sources(
    ${CMAKE_CURRENT_SOURCE_DIR}/font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ljson.c)
if (MSVC)
    source_group("Source Files\\font" FILES ${CMAKE_CURRENT_SOURCE_DIR}/font.c)
    source_group("Source Files\\json" FILES ${CMAKE_CURRENT_SOURCE_DIR}/ljson.c)
endif ()

# NOTE: image.c and ctypes.c are not compiled today (they are not registered
# in src/selene.c).  Wire them here once they are part of the runtime:
#   selene_add_module_sources(${CMAKE_CURRENT_SOURCE_DIR}/image.c)
#   selene_add_module_sources(${CMAKE_CURRENT_SOURCE_DIR}/ctypes.c)
