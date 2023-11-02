#include "gl.h"

static MODULE_FUNCTION(VertexArray, create) {
    NEW_UDATA(VertexArray, vao);
#if !defined(__EMSCRIPTEN__)
    glGenVertexArrays(1, vao);
#endif
    return 1;
}

static MODULE_FUNCTION(VertexArray, unbind) {
#if !defined(__EMSCRIPTEN__)
    glBindVertexArray(0);
#endif
    return 0;
}

static MODULE_FUNCTION(VertexArray, enable) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glEnableVertexAttribArray(attrib);
    return 0;
}

static MODULE_FUNCTION(VertexArray, disable) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    glDisableVertexAttribArray(attrib);
    return 0;
}

static MODULE_FUNCTION(VertexArray, attribPointer) {
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

static META_FUNCTION(VertexArray, bind) {
    INIT_ARG();
    TEST_UDATA(VertexArray, vao);
#if !defined(__EMSCRIPTEN__)
    if (vao) glBindVertexArray(*vao);
    else glBindVertexArray(0);
#endif
    return 0;
}

static META_FUNCTION(VertexArray, destroy) {
    CHECK_META(VertexArray);
#if !defined(__EMSCRIPTEN__)
    glDeleteVertexArrays(1, self);
#endif
    return 0;
}

BEGIN_META(VertexArray) {
    BEGIN_REG()
        REG_FIELD(VertexArray, create),
        REG_FIELD(VertexArray, unbind),
        REG_FIELD(VertexArray, enable),
        REG_FIELD(VertexArray, disable),
        REG_FIELD(VertexArray, attribPointer),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(VertexArray, bind),
        REG_META_FIELD(VertexArray, destroy),
    END_REG()
    NEW_META(VertexArray, _reg, _index_reg);
    return 1;
}
