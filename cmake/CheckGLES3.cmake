# ---------------------------------------------------------------------------
# OpenGL ES 3 feature probe (Emscripten).
#
# Emscripten uses GLES3 headers, which declare VAO/instancing/UBO but only
# glClearDepthf (glClearDepth is desktop GL only).  Included only when
# EMSCRIPTEN is defined (see src/CMakeLists.txt).
# ---------------------------------------------------------------------------
include(CheckCSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state()
set(CMAKE_REQUIRED_FLAGS "-Werror=implicit-function-declaration")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
check_c_source_compiles("
#include <GLES3/gl3.h>
int main(void) {
    glClearDepth(1.0);
    return 0;
}" SELENE_PROBE_GLES_CLEARDEPTH)
cmake_pop_check_state()
if (NOT SELENE_PROBE_GLES_CLEARDEPTH)
    message("Emscripten GLES3: glClearDepth not declared — aliasing to glClearDepthf")
    add_definitions(-DSELENE_GLES_GLCLEARDEPTH_F)
endif ()
