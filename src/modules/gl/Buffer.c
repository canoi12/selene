#include "gl.h"

static MODULE_FUNCTION(Buffer, New) {
    NEW_UDATA(Buffer, buffer);
    glGenBuffers(1, buffer);
    return 1;
}

static MODULE_FUNCTION(Buffer, Bind) {
    INIT_ARG();
    CHECK_INTEGER(target);
    TEST_UDATA(Buffer, buf);
    if (buf) glBindBuffer(target, *buf);
    else glBindBuffer(target, 0);
    return 0;
}

static MODULE_FUNCTION(Buffer, Data) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(size);
    CHECK_INTEGER(usage);
    TEST_UDATA(Data, data);
    if (data) glBufferData(target, size, data->data, usage);
    else glBufferData(target, size, NULL, usage);
    return 0;
}

static MODULE_FUNCTION(Buffer, SubData) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(start);
    CHECK_INTEGER(size);
    CHECK_UDATA(Data, data);
    glBufferSubData(target, start, size, data->data);
    return 0;
}

// Meta

static META_FUNCTION(Buffer, Delete) {
    CHECK_META(Buffer);
    glDeleteBuffers(1, self);
    return 0;
}

BEGIN_META(Buffer) {
    BEGIN_REG()
        REG_FIELD(Buffer, New),
        REG_FIELD(Buffer, Bind),
        REG_FIELD(Buffer, Data),
        REG_FIELD(Buffer, SubData),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Buffer, Delete),
    END_REG()
    NEW_META(Buffer, _reg, _index_reg);
    return 1;
}
