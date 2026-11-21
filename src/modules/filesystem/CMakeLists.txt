# filesystem module: virtual filesystem / zip mounting and Lua binding.
file(GLOB FS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
selene_add_module_sources(${FS_SOURCES})
if (MSVC)
    source_group("Source Files\\filesystem" FILES ${FS_SOURCES})
endif ()
