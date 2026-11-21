# audio module: playback system, decoder and Lua binding.
file(GLOB AUDIO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
selene_add_module_sources(${AUDIO_SOURCES})
if (MSVC)
    source_group("Source Files\\audio" FILES ${AUDIO_SOURCES})
endif ()
