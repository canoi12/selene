#include "gl.h"

static MODULE_FUNCTION(Program, create) {
    NEW_UDATA(Program, p);
    *p = glCreateProgram();
    return 1;
}

static MODULE_FUNCTION(Program, use) {
    INIT_ARG();
    TEST_UDATA(Program, p);
    if (p) glUseProgram(*p);
    else glUseProgram(0);
    return 0;
}

// Meta

static META_FUNCTION(Program, destroy) {
    CHECK_META(Program);
    glDeleteProgram(*self);
    return 0;
}

static META_FUNCTION(Program, attachShader) {
    CHECK_META(Program);
    int args = lua_gettop(LUA_STATE_NAME);
    while (arg <= args) {
        CHECK_UDATA(Shader, s);
        glAttachShader(*self, *s);
    }
    return 0;
}

static META_FUNCTION(Program, link) {
    CHECK_META(Program);
    glLinkProgram(*self);
    int success = 0;
    glGetProgramiv(*self, GL_LINK_STATUS, &success);
    if (!success) {
        int len;
        glGetProgramiv(*self, GL_INFO_LOG_LENGTH, &len);
    #if defined(OS_WIN)
        if (len >= 2048)
            len = 2048;
        char log[2049];
    #else
        char log[len+1];
    #endif
        glGetProgramInfoLog(*self, len, NULL, log);
        log[len] = '\0';
        return luaL_error(L, "Failed to link program: %s\n", log);
    }
    return 0;
}

static META_FUNCTION(Program, getAttribLocation) {
    CHECK_META(Program);
    CHECK_STRING(name);
    PUSH_INTEGER(glGetAttribLocation(*self, name));
    return 1;
}

static META_FUNCTION(Program, getUniformLocation) {
    CHECK_META(Program);
    CHECK_STRING(name);
    PUSH_INTEGER(glGetUniformLocation(*self, name));
    return 1;
}

BEGIN_META(Program) {
    BEGIN_REG()
        REG_FIELD(Program, create),
        REG_FIELD(Program, use),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Program, destroy),
        REG_META_FIELD(Program, attachShader),
        REG_META_FIELD(Program, link),
        REG_META_FIELD(Program, getAttribLocation),
        REG_META_FIELD(Program, getUniformLocation),
    END_REG()
    NEW_META(Program, _reg, _index_reg);
    return 1;
}
