# ---------------------------------------------------------------------------
# selene-editor: run / pack / edit front-end over the runtime (desktop only).
# Included from src/CMakeLists.txt.
# ---------------------------------------------------------------------------
file(GLOB EDITOR_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
add_executable(${PROJECT_NAME}Editor ${EDITOR_SOURCES})
message(${LIBRARIES})
target_link_libraries(${PROJECT_NAME}Editor "${PROJECT_NAME}Lib" ${LIBRARIES})
target_compile_features(${PROJECT_NAME}Editor PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME}Editor PROPERTIES OUTPUT_NAME ${PROJECT_NAME}-editor)
if (MSVC)
    target_compile_definitions(${PROJECT_NAME}Editor PRIVATE -DSELENE_IMPORT_DLL)
endif ()
if (HAS_LUA51)
    target_link_libraries(${PROJECT_NAME}Editor lua51)
endif ()
if (WIN32)
    target_sources(${PROJECT_NAME}Editor PRIVATE "${CMAKE_SOURCE_DIR}/misc/cross/windres/ICON.rc")
endif ()
if (DEFINED MINGW)
    target_link_libraries(${PROJECT_NAME}Editor mingw32 opengl32 glu32)
endif ()
