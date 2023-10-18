#include "gl.h"

static MODULE_FUNCTION(Texture, New) {
    NEW_UDATA(Texture, tex);
    glGenTextures(1, tex);
    return 1;
}

static MODULE_FUNCTION(Texture, Bind) {
    INIT_ARG();
    CHECK_INTEGER(target);
    TEST_UDATA(Texture, tex);
    if (tex) glBindTexture(target, *tex);
    else glBindTexture(target, 0);
    return 0;
}

static MODULE_FUNCTION(Texture, Image2D) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(internal);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    CHECK_INTEGER(format);
    CHECK_INTEGER(type);
    void* data = NULL;
    if (lua_type(L, arg) == LUA_TUSERDATA) {
        CHECK_UDATA(Data, dt);
        data = dt->data;
    }
    glTexImage2D(target, 0, internal, width, height, 0, format, type, data);
    return 0;
}

static MODULE_FUNCTION(Texture, SubImage2D) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(xoffset);
    CHECK_INTEGER(yoffset);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    CHECK_INTEGER(format);
    CHECK_INTEGER(type);
    void* data = NULL;
    if (lua_type(L, arg) == LUA_TUSERDATA) {
        CHECK_UDATA(Data, dt);
        data = dt->data;
    }
    glTexSubImage2D(target, 0, xoffset, yoffset, width, height, format, type, data);
    return 0;
}

static MODULE_FUNCTION(Texture, Parameteri) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(pname);
    CHECK_INTEGER(param);
    glTexParameteri(target, pname, param);
    return 0;
}

// Meta

static META_FUNCTION(Texture, Delete) {
    CHECK_META(Texture);
    glDeleteTextures(1, self);
    return 0;
}

BEGIN_META(Texture) {
    BEGIN_REG()
        REG_FIELD(Texture, New),
        REG_FIELD(Texture, Bind),
        REG_FIELD(Texture, Image2D),
        REG_FIELD(Texture, SubImage2D),
        REG_FIELD(Texture, Parameteri),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Texture, Delete),
    END_REG()
    NEW_META(Texture, _reg, _index_reg);
    return 1;
}
