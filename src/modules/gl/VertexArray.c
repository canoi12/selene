#include "gl.h"

static MODULE_FUNCTION(VertexArray, New) {
    NEW_UDATA(VertexArray, vao);
#if !defined(__EMSCRIPTEN__)
    glGenVertexArrays(1, vao);
#endif
    return 1;
}

static MODULE_FUNCTION(VertexArray, Unbind) {
#if !defined(__EMSCRIPTEN__)
    glBindVertexArray(0);
#endif
    return 0;
}

static MODULE_FUNCTION(VertexArray, Enable) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glEnableVertexAttribArray(attrib);
    return 0;
}

static MODULE_FUNCTION(VertexArray, Disable) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glDisableVertexAttribArray(attrib);
    return 0;
}

static MODULE_FUNCTION(VertexArray, AttribPointer) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    CHECK_INTEGER(size);
    CHECK_INTEGER(type);
    GET_BOOLEAN(normalized);
    CHECK_INTEGER(stride);
    CHECK_INTEGER(offset);
    glVertexAttribPointer(attrib, size, type, normalized, stride, (void*)offset);
    return 0;
}

// Meta

static META_FUNCTION(VertexArray, Bind) {
    INIT_ARG();
    TEST_UDATA(VertexArray, vao);
#if !defined(__EMSCRIPTEN__)
    if (vao) glBindVertexArray(*vao);
    else glBindVertexArray(0);
#endif
    return 0;
}

static META_FUNCTION(VertexArray, Delete) {
    CHECK_META(VertexArray);
#if !defined(__EMSCRIPTEN__)
    glDeleteVertexArrays(1, self);
#endif
    return 0;
}

BEGIN_META(VertexArray) {
    BEGIN_REG()
        REG_FIELD(VertexArray, New),
        REG_FIELD(VertexArray, Unbind),
        REG_FIELD(VertexArray, Enable),
        REG_FIELD(VertexArray, Disable),
        REG_FIELD(VertexArray, AttribPointer),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(VertexArray, Bind),
        REG_META_FIELD(VertexArray, Delete),
    END_REG()
    NEW_META(VertexArray, _reg, _index_reg);
    return 1;
}
