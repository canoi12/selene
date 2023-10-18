#include "gl.h"

extern MODULE_FUNCTION(Texture, meta);
extern MODULE_FUNCTION(Framebuffer, meta);
extern MODULE_FUNCTION(Buffer, meta);
extern MODULE_FUNCTION(VertexArray, meta);
extern MODULE_FUNCTION(Shader, meta);
extern MODULE_FUNCTION(Program, meta);

struct GLInfo {
    uint8_t major, minor;
    uint16_t glsl;
    uint16_t es;
};

static struct GLInfo _gl_info;

static MODULE_FUNCTION(gl, LoadGlad) {
    INIT_ARG();
    CHECK_LUDATA(void, proc_fn);
#if !defined(__EMSCRIPTEN__)
    if (!gladLoadGLLoader((GLADloadproc)proc_fn)) {
        return luaL_error(L, "Failed to init GLAD");
    }
#endif
    return 0;
}

static MODULE_FUNCTION(gl, Viewport) {
    int view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        view[i] = (int)luaL_checkinteger(L, 1+i);
    }
    glViewport(view[0], view[1], view[2], view[3]);
    return 0;
}

static MODULE_FUNCTION(gl, ClearDepth) {
    INIT_ARG();
    OPT_NUMBER(float, depth, 1.f);
    #if !defined(__EMSCRIPTEN__)
        glClearDepth(depth);
    #else
        glClearDepthf(depth);
    #endif
    return 0;
}

static MODULE_FUNCTION(gl, ClearColor) {
    float color[4] = { 0.f, 0.f, 0.f, 1.f };
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++)
        color[i] = (float)lua_tonumber(L, i+1);
    glClearColor(color[0], color[1], color[2], color[3]);
    return 0;
}

static MODULE_FUNCTION(gl, Clear) {
    GLenum flags = 0;
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        flags |= luaL_checkinteger(L, 1+i);
    }
    glClear(flags);
    return 0;
}

static MODULE_FUNCTION(gl, Enable) {
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++)
        glEnable((GLenum)luaL_checkinteger(L, 1+i));
    return 0;
}

static MODULE_FUNCTION(gl, Disable) {
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++)
        glDisable((GLenum)luaL_checkinteger(L, 1+i));
    return 0;
}

static MODULE_FUNCTION(gl, Scissor) {
    INIT_ARG();
    CHECK_INTEGER(x);
    CHECK_INTEGER(y);
    CHECK_INTEGER(w);
    CHECK_INTEGER(h);
    glScissor(x, y, w, h);
    return 0;
}

static MODULE_FUNCTION(gl, BlendFunc) {
    INIT_ARG();
    OPT_INTEGER(sfn, GL_SRC_ALPHA);
    OPT_INTEGER(dfn, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(sfn, dfn);
    return 0;
}

static MODULE_FUNCTION(gl, DrawArrays) {
    uint16_t mode = (uint16_t)luaL_checkinteger(L, 1);
    uint32_t start = (uint32_t)luaL_checkinteger(L, 2);
    uint32_t count = (uint32_t)luaL_checkinteger(L, 3);
    glDrawArrays(mode, start, count);
    return 0;
}

static MODULE_FUNCTION(gl, DrawElements) {
    uint16_t mode = (uint16_t)luaL_checkinteger(L, 1);
    uint32_t start = (uint32_t)luaL_checkinteger(L, 2);
    uint32_t count = (uint32_t)luaL_checkinteger(L, 3);
    uint16_t _type = (uint16_t)luaL_checkinteger(L, 4);
    glDrawElements(mode, start, _type, (void*)start);
    return 0;
}

/* Uniforms */

static MODULE_FUNCTION(gl, Uniform1fv) {
    int location = (int)luaL_checkinteger(L, 1);
    int args = (int)lua_gettop(L) - 1;
#if defined(OS_WIN)
    float values[1024];
#else
    float values[args];
#endif
    for (int i = 0; i < args; i++) {
        values[i] = (float)luaL_checknumber(L, 2+i);
    }
    glUniform1fv(location, args, values);
    return 0;
}

static MODULE_FUNCTION(gl, Uniform2fv) {
    int location = (int)luaL_checkinteger(L, 1);
    int args = lua_gettop(L) - 2;
    int size = args * 4 * 2;
#if defined(OS_WIN)
    float values[1024];
#else
    float values[args*2];
#endif
    float* v = values;
    for (int i = 0; i < args; i++) {
        int index = i + 2;
        if (lua_type(L, index) != LUA_TTABLE)
            return luaL_argerror(L, index, "Must be a table");
        lua_pushnil(L);
        int j = 0;
        while (lua_next(L, index) != 0) {
            v[j++] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 2;
    }
    glUniform2fv(location, args, values);
    return 0;
}

static MODULE_FUNCTION(gl, Uniform3fv) {
    int location = (int)luaL_checkinteger(L, 1);
    int args = lua_gettop(L) - 2;
    int size = args * 4 * 3;
#if defined(OS_WIN)
    float values[1024];
#else
    float values[args*3];
#endif
    float *v = values;
    for (int i = 0; i < args; i++) {
        int index = i + 2;
        if (lua_type(L, index) != LUA_TTABLE)
            return luaL_argerror(L, index, "Must be a table");
        lua_pushnil(L);
        int j = 0;
        while (lua_next(L, index) != 0) {
            v[j++] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 3;
    }
    glUniform3fv(location, args, values);
    return 0;
}

static MODULE_FUNCTION(gl, Uniform4fv) {
    int location = (int)luaL_checkinteger(L, 1);
    int args = lua_gettop(L) - 2;
    int size = args * 4 * 4;
#if defined(OS_WIN)
    float values[1024];
#else
    float values[args*4];
#endif
    float *v = values;
    for (int i = 0; i < args; i++) {
        int index = i + 2;
        if (lua_type(L, index) != LUA_TTABLE)
            return luaL_argerror(L, index, "Must be a table");
        lua_pushnil(L);
        int j = 0;
        while (lua_next(L, index) != 0) {
            v[j++] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 4;
    }
    glUniform4fv(location, args, values);
    return 0;
}

static MODULE_FUNCTION(gl, UniformMatrix4fv) {
    INIT_ARG();
    CHECK_INTEGER(location);
    CHECK_INTEGER(count);
    GET_BOOLEAN(normalize);
    mat4x4* m = luaL_checkudata(L, arg++, "Mat4");
    glUniformMatrix4fv(location, count, normalize, **m);
    return 0;
}

static const BEGIN_ENUM(gl)
    // Clear
    ENUM_FIELD(COLOR_BUFFER_BIT, GL_),
    ENUM_FIELD(DEPTH_BUFFER_BIT, GL_),
    ENUM_FIELD(STENCIL_BUFFER_BIT, GL_),
    // Types
    ENUM_FIELD(BYTE, GL_),
    ENUM_FIELD(UNSIGNED_BYTE, GL_),
    ENUM_FIELD(SHORT, GL_),
    ENUM_FIELD(UNSIGNED_SHORT, GL_),
    ENUM_FIELD(INT, GL_),
    ENUM_FIELD(UNSIGNED_INT, GL_),
    ENUM_FIELD(FLOAT, GL_),
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(DOUBLE, GL_),
#endif
    // Enable
    ENUM_FIELD(DEPTH_TEST, GL_),
    ENUM_FIELD(STENCIL_TEST, GL_),
    ENUM_FIELD(SCISSOR_TEST, GL_),
    ENUM_FIELD(BLEND, GL_),
    ENUM_FIELD(CULL_FACE, GL_),
    // Pixel
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(RED, GL_),
    ENUM_FIELD(RG, GL_),
    ENUM_FIELD(BGR, GL_),
    ENUM_FIELD(BGRA, GL_),
#endif
    ENUM_FIELD(RGB, GL_),
    ENUM_FIELD(RGBA, GL_),
    ENUM_FIELD(DEPTH_COMPONENT, GL_),
    ENUM_FIELD(DEPTH_COMPONENT16, GL_),
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(DEPTH_COMPONENT24, GL_),
    ENUM_FIELD(DEPTH_COMPONENT32, GL_),
    ENUM_FIELD(DEPTH_COMPONENT32F, GL_),
    ENUM_FIELD(DEPTH32F_STENCIL8, GL_),
    ENUM_FIELD(DEPTH24_STENCIL8, GL_),
#endif
    // Funcs
    ENUM_FIELD(ZERO, GL_),
    ENUM_FIELD(ONE, GL_),
    ENUM_FIELD(SRC_COLOR, GL_),
    ENUM_FIELD(ONE_MINUS_SRC_COLOR, GL_),
    ENUM_FIELD(SRC_ALPHA, GL_),
    ENUM_FIELD(ONE_MINUS_SRC_ALPHA, GL_),
    ENUM_FIELD(DST_ALPHA, GL_),
    ENUM_FIELD(ONE_MINUS_DST_ALPHA, GL_),
    ENUM_FIELD(DST_COLOR, GL_),
    ENUM_FIELD(ONE_MINUS_DST_COLOR, GL_),
    ENUM_FIELD(SRC_ALPHA_SATURATE, GL_),
    ENUM_FIELD(CONSTANT_COLOR, GL_),
    ENUM_FIELD(ONE_MINUS_CONSTANT_COLOR, GL_),
    ENUM_FIELD(CONSTANT_ALPHA, GL_),
    ENUM_FIELD(ONE_MINUS_CONSTANT_ALPHA, GL_),
    // Draw mode
    ENUM_FIELD(POINTS, GL_),
    ENUM_FIELD(LINES, GL_),
    ENUM_FIELD(TRIANGLES, GL_),
    // Texture targers
    ENUM_FIELD(TEXTURE_2D, GL_),
    ENUM_FIELD(TEXTURE_CUBE_MAP, GL_),
    ENUM_FIELD(TEXTURE_CUBE_MAP_NEGATIVE_X, GL_),
    ENUM_FIELD(TEXTURE_CUBE_MAP_POSITIVE_X, GL_),
    ENUM_FIELD(TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_),
    ENUM_FIELD(TEXTURE_CUBE_MAP_POSITIVE_Y, GL_),
    ENUM_FIELD(TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_),
    ENUM_FIELD(TEXTURE_CUBE_MAP_POSITIVE_Z, GL_),
    // Texture params
    ENUM_FIELD(TEXTURE_MIN_FILTER, GL_),
    ENUM_FIELD(TEXTURE_MAG_FILTER, GL_),
    ENUM_FIELD(TEXTURE_WRAP_S, GL_),
    ENUM_FIELD(TEXTURE_WRAP_T, GL_),
    ENUM_FIELD(NEAREST, GL_),
    ENUM_FIELD(LINEAR, GL_),
    ENUM_FIELD(REPEAT, GL_),
    ENUM_FIELD(CLAMP_TO_EDGE, GL_),
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(CLAMP_TO_BORDER, GL_),
#endif
    // Framebuffer targets
    ENUM_FIELD(FRAMEBUFFER, GL_),
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(DRAW_FRAMEBUFFER, GL_),
    ENUM_FIELD(READ_FRAMEBUFFER, GL_),
    ENUM_FIELD(DEPTH_STENCIL_ATTACHMENT, GL_),
#endif
    ENUM_FIELD(COLOR_ATTACHMENT0, GL_),
    ENUM_FIELD(DEPTH_ATTACHMENT, GL_),
    ENUM_FIELD(STENCIL_ATTACHMENT, GL_),
    // Buffer
    ENUM_FIELD(ARRAY_BUFFER, GL_),
    ENUM_FIELD(ELEMENT_ARRAY_BUFFER, GL_),
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(UNIFORM_BUFFER, GL_),
    ENUM_FIELD(DYNAMIC_READ, GL_),
    ENUM_FIELD(DYNAMIC_COPY, GL_),
    ENUM_FIELD(STATIC_READ, GL_),
    ENUM_FIELD(STATIC_COPY, GL_),
    ENUM_FIELD(STREAM_READ, GL_),
    ENUM_FIELD(STREAM_COPY, GL_),
#endif
    ENUM_FIELD(DYNAMIC_DRAW, GL_),
    ENUM_FIELD(STATIC_DRAW, GL_),
    ENUM_FIELD(STREAM_DRAW, GL_),
    // Shader
    ENUM_FIELD(FRAGMENT_SHADER, GL_),
    ENUM_FIELD(VERTEX_SHADER, GL_),
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(GEOMETRY_SHADER, GL_),
#endif
END_ENUM()

BEGIN_MODULE(gl) {
    BEGIN_REG()
        REG_FIELD(gl, LoadGlad),
        REG_FIELD(gl, ClearDepth),
        REG_FIELD(gl, ClearColor),
        REG_FIELD(gl, Clear),
        REG_FIELD(gl, Viewport),
        REG_FIELD(gl, Enable),
        REG_FIELD(gl, Disable),
        REG_FIELD(gl, BlendFunc),
        REG_FIELD(gl, DrawArrays),
        REG_FIELD(gl, DrawElements),
        /* Uniform */
        REG_FIELD(gl, Uniform1fv),
        REG_FIELD(gl, Uniform2fv),
        REG_FIELD(gl, Uniform3fv),
        REG_FIELD(gl, Uniform4fv),
        REG_FIELD(gl, UniformMatrix4fv),
    END_REG()
    luaL_newlib(LUA_STATE_NAME, _reg);
    LOAD_META(Texture);
    LOAD_META(Framebuffer);
    LOAD_META(Buffer);
    LOAD_META(VertexArray);
    LOAD_META(Shader);
    LOAD_META(Program);
    LOAD_ENUM(gl);
    return 1;
}
