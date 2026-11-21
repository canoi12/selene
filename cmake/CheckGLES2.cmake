# ---------------------------------------------------------------------------
# OpenGL ES 2 feature probes (Android).
#
# clang only WARNS on implicit function declarations in C99, so every probe
# runs with -Werror=implicit-function-declaration.  Header-declaration probes
# are compile-only (STATIC_LIBRARY); extension entry points also get a LINK
# probe (executable against GLESv2) because a declaration alone does not mean
# the symbol is exported by the Android lib.
#
# Included only for ANDROID builds (see src/CMakeLists.txt).  The resulting
# SELENE_GLES_*/SELENE_NO_* definitions are consumed by
# src/modules/renderer/drivers/renderer_driver_opengl.c.
# ---------------------------------------------------------------------------
include(CheckCSourceCompiles)
include(CMakePushCheckState)

set(OPENGL_LIBRARIES GLESv1_CM GLESv2)

cmake_push_check_state()
set(CMAKE_REQUIRED_FLAGS "-Werror=implicit-function-declaration")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
check_c_source_compiles("
#include <GLES2/gl2.h>
int main(void) {
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glDeleteVertexArrays(1, &vao);
    return 0;
}" SELENE_PROBE_GLES_VAO_CORE)
if (NOT SELENE_PROBE_GLES_VAO_CORE)
    check_c_source_compiles("
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#ifndef GL_OES_vertex_array_object
#error GL_OES_vertex_array_object is not defined in gl2ext.h
#endif
int main(void) {
    GLuint vao = 0;
    glGenVertexArraysOES(1, &vao);
    glBindVertexArrayOES(vao);
    glDeleteVertexArraysOES(1, &vao);
    return 0;
}" SELENE_PROBE_GLES_VAO_OES_DECL)
endif ()
check_c_source_compiles("
#include <GLES2/gl2.h>
int main(void) {
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 2);
    glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0, 2);
    return 0;
}" SELENE_PROBE_GLES_INSTANCING_CORE)
if (NOT SELENE_PROBE_GLES_INSTANCING_CORE)
    check_c_source_compiles("
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
int main(void) {
    glDrawArraysInstancedANGLE(GL_TRIANGLES, 0, 3, 2);
    glDrawElementsInstancedANGLE(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0, 2);
    return 0;
}" SELENE_PROBE_GLES_INSTANCING_ANGLE)
endif ()
check_c_source_compiles("
#include <GLES2/gl2.h>
int main(void) {
    glClearDepth(1.0);
    return 0;
}" SELENE_PROBE_GLES_CLEARDEPTH)
check_c_source_compiles("
#include <GLES2/gl2.h>
int main(void) {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    glUniformBlockBinding(0, 0, 0);
    (void)glGetUniformBlockIndex(0, \"x\");
    return 0;
}" SELENE_PROBE_GLES_UBO)
# Link probes: is the extension entry point actually exported by libGLESv2?
set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
set(CMAKE_REQUIRED_LIBRARIES GLESv2)
if (SELENE_PROBE_GLES_VAO_OES_DECL)
    check_c_source_compiles("
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
int main(void) {
    GLuint vao = 0;
    glGenVertexArraysOES(1, &vao);
    glBindVertexArrayOES(vao);
    glDeleteVertexArraysOES(1, &vao);
    return 0;
}" SELENE_PROBE_GLES_VAO_OES_LINK)
endif ()
if (SELENE_PROBE_GLES_INSTANCING_ANGLE)
    check_c_source_compiles("
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
int main(void) {
    glDrawArraysInstancedANGLE(GL_TRIANGLES, 0, 3, 2);
    glDrawElementsInstancedANGLE(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0, 2);
    return 0;
}" SELENE_PROBE_GLES_INSTANCING_LINK)
endif ()
cmake_pop_check_state()
if (SELENE_PROBE_GLES_VAO_CORE)
    message("Android GLES: VAO entry points declared in GLES2 headers")
elseif (SELENE_PROBE_GLES_VAO_OES_LINK)
    message("Android GLES: VAO available via GL_OES_vertex_array_object — defining SELENE_GLES_VAO_OES")
    add_definitions(-DSELENE_GLES_VAO_OES)
else ()
    message("Android GLES: no VAO declarations found — defining SELENE_NO_VAO")
    add_definitions(-DSELENE_NO_VAO)
endif ()
if (SELENE_PROBE_GLES_INSTANCING_CORE)
    message("Android GLES: instancing entry points declared in GLES2 headers")
elseif (SELENE_PROBE_GLES_INSTANCING_LINK)
    message("Android GLES: instancing available via GL_ANGLE_instanced_arrays — defining SELENE_GLES_INSTANCING_OES")
    add_definitions(-DSELENE_GLES_INSTANCING_OES)
else ()
    message("Android GLES: no instancing declarations found — defining SELENE_NO_INSTANCING")
    add_definitions(-DSELENE_NO_INSTANCING)
endif ()
if (NOT SELENE_PROBE_GLES_CLEARDEPTH)
    message("Android GLES: glClearDepth not declared — aliasing to glClearDepthf")
    add_definitions(-DSELENE_GLES_GLCLEARDEPTH_F)
endif ()
if (NOT SELENE_PROBE_GLES_UBO)
    message("Android GLES: uniform buffer objects not declared — defining SELENE_NO_UBO")
    add_definitions(-DSELENE_NO_UBO)
endif ()
