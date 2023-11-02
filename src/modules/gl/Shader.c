#include "gl.h"

static MODULE_FUNCTION(Shader, create) {
    INIT_ARG();
    CHECK_INTEGER(tp);
    NEW_UDATA(Shader, shader);
    *shader = glCreateShader(tp);
    return 1;
}

// Meta

static META_FUNCTION(Shader, destroy) {
    CHECK_META(Shader);
    glDeleteShader(*self);
    return 0;
}

static META_FUNCTION(Shader, source) {
    CHECK_META(Shader);
    CHECK_STRING(source);
    glShaderSource(*self, 1, &source, NULL);
    return 0;
}

static META_FUNCTION(Shader, compile) {
    CHECK_META(Shader);
    glCompileShader(*self);
    int success = 0;
    glGetShaderiv(*self, GL_COMPILE_STATUS, &success);
    if (!success) {
        int len;
        glGetShaderiv(*self, GL_INFO_LOG_LENGTH, &len);
    #if defined(OS_WIN)
        if (len >= 2048)
            len = 2048;
        char log[2049];
    #else
        char log[len+1];
    #endif
        glGetShaderInfoLog(*self, len, NULL, log);
        log[len] = '\0';
        return luaL_error(L, "Failed to compile shader: %s\n", log);
    }
    return 0;
}

BEGIN_META(Shader) {
    BEGIN_REG()
        REG_FIELD(Shader, create),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Shader, destroy),
        REG_META_FIELD(Shader, source),
        REG_META_FIELD(Shader, compile),
    END_REG()
    NEW_META(Shader, _reg, _index_reg);
    return 1;
}
