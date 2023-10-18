#include "gl.h"

static MODULE_FUNCTION(Shader, New) {
    INIT_ARG();
    CHECK_INTEGER(tp);
    NEW_UDATA(Shader, shader);
    *shader = glCreateShader(tp);
    return 1;
}

// Meta

static META_FUNCTION(Shader, Delete) {
    CHECK_META(Shader);
    glDeleteShader(*self);
    return 0;
}

static META_FUNCTION(Shader, Source) {
    CHECK_META(Shader);
    CHECK_STRING(source);
    glShaderSource(*self, 1, &source, NULL);
    return 0;
}

static META_FUNCTION(Shader, Compile) {
    CHECK_META(Shader);
    glCompileShader(*self);
    int success = 0;
    glGetShaderiv(*self, GL_COMPILE_STATUS, &success);
    if (!success) {
        int len;
        glGetShaderiv(*self, GL_INFO_LOG_LENGTH, &len);
    #if defined(OS_WIN)
        if (len+1 > s_aux_data->size) {
            s_aux_data->size = (len+1)*2;
            s_aux_data->data = realloc(s_aux_data->data, s_aux_data->size);
        }
        char* log = s_aux_data->data;
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
        REG_FIELD(Shader, New),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Shader, Delete),
        REG_META_FIELD(Shader, Source),
        REG_META_FIELD(Shader, Compile),
    END_REG()
    NEW_META(Shader, _reg, _index_reg);
    return 1;
}
