# ---------------------------------------------------------------------------
# Bundled third-party libraries.  Each one becomes a static target that the
# selene library links against (the target names are part of the linker
# interface assembled in src/CMakeLists.txt).
# ---------------------------------------------------------------------------

# --- cglm (vector/matrix math) ---------------------------------------------
file(GLOB CGLM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/cglm/src/*.c)
add_library(cglm STATIC ${CGLM_FILES})
target_compile_definitions(cglm PRIVATE -DCGLM_STATIC)
target_include_directories(cglm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cglm/include)
set_target_properties(cglm PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --- miniz (zip / deflate) --------------------------------------------------
file(GLOB MINIZ_FILES ${CMAKE_CURRENT_SOURCE_DIR}/miniz/miniz.c)
add_library(miniz STATIC ${MINIZ_FILES})
target_include_directories(miniz PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/miniz)
set_target_properties(miniz PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --- glad (OpenGL loader) ---------------------------------------------------
# Linked everywhere except Emscripten, which gets the GL entry points from its
# GLES3 headers (see src/CMakeLists.txt).
add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c)
target_include_directories(glad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
