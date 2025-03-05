#include "minigl.h"

#if defined(OS_EMSCRIPTEN) || defined(OS_ANDROID)
#include <GLES2/gl2.h>
#define USE_GL_LIB
#endif

#define DEFINE_UNIFORM(type, suffix)\
selUniform1##suffix sel_Uniform1##suffix = NULL;\
selUniform2##suffix sel_Uniform2##suffix = NULL;\
selUniform3##suffix sel_Uniform3##suffix = NULL;\
selUniform4##suffix sel_Uniform4##suffix = NULL;\
selUniform##suffix##v sel_Uniform1##suffix##v = NULL;\
selUniform##suffix##v sel_Uniform2##suffix##v = NULL;\
selUniform##suffix##v sel_Uniform3##suffix##v = NULL;\
selUniform##suffix##v sel_Uniform4##suffix##v = NULL;


#define UNIFORM_PROC(suffix)\
{(void**)&sel_Uniform1##suffix, {"glUniform1"#suffix, NULL, NULL}},\
{(void**)&sel_Uniform2##suffix, {"glUniform2"#suffix, NULL, NULL}},\
{(void**)&sel_Uniform3##suffix, {"glUniform3"#suffix, NULL, NULL}},\
{(void**)&sel_Uniform3##suffix, {"glUniform4"#suffix, NULL, NULL}},\
{(void**)&sel_Uniform4##suffix, {"glUniform1"#suffix"v", NULL, NULL}},\
{(void**)&sel_Uniform4##suffix, {"glUniform2"#suffix"v", NULL, NULL}},\
{(void**)&sel_Uniform4##suffix, {"glUniform3"#suffix"v", NULL, NULL}},\
{(void**)&sel_Uniform4##suffix, {"glUniform4"#suffix"v", NULL, NULL}}

#define FUNC_LOADER_FIELD(sel_func, gl_func)\
{(void**)&(sel_func), {#gl_func, NULL, NULL}}

#if !defined(USE_GL_LIB)
#define CALL_GL(name, ...)\
sel_##name(__VA_ARGS__)
#else
#define CALL_GL(name, ...)\
gl##name(__VA_ARGS__)
#endif

selGetString sel_GetString = NULL;
selFuncIntParam sel_Clear = NULL;
selFuncFloat4Param sel_ClearColor = NULL;

selEnable sel_Enable = NULL;
selDisable sel_Disable = NULL;

selFuncInt2Param sel_BlendFunc = NULL;
selFuncIntParam sel_BlendEquation = NULL;

selDrawArrays sel_DrawArrays = NULL;
selDrawElements sel_DrawElements = NULL;

selGetLocation sel_GetUniformLocation = NULL;
selGetLocation sel_GetAttribLocation = NULL;

DEFINE_UNIFORM(float, f)
DEFINE_UNIFORM(int, i)
selUniformMatrix4fv sel_UniformMatrix4fv = NULL;

/* Vertex Array */
DECLARE_GLOBJECT(VertexArray, selBindObject);
selEnableVertexAttribArray sel_EnableVertexAttribArray = NULL;
selDisableVertexAttribArray sel_DisableVertexAttribArray = NULL;
selVertexAttribPointer sel_VertexAttribPointer = NULL;

/* Buffer */
DECLARE_GLOBJECT(Buffer, selBindObjectTarget);
selBufferData sel_BufferData = NULL;
selBufferSubData sel_BufferSubData = NULL;

/* Texture */
DECLARE_GLOBJECT(Texture, selBindObjectTarget);
selTexImage2D sel_TexImage2D = NULL;
selParameteri sel_TexParameteri = NULL;
selParameterf sel_TexParameterf = NULL;

DECLARE_GLOBJECT(Framebuffer, selBindObjectTarget);

selFramebufferTexture2D sel_FramebufferTexture2D = NULL;

/* Program */

selCreateProgram sel_CreateProgram = NULL;
selBindObject sel_UseProgram = NULL;
selDeleteObject sel_DeleteProgram = NULL;
selAttachShader sel_AttachShader = NULL;
selLinkProgram sel_LinkProgram = NULL;
selGetObjectiv sel_GetProgramiv = NULL;
selGetInfoLog sel_GetProgramInfoLog = NULL;

/* Shader */
selCreateShader sel_CreateShader = NULL;
selDeleteObject sel_DeleteShader = NULL;

selShaderSource sel_ShaderSource = NULL;
selCompileShader sel_CompileShader = NULL;
selGetObjectiv sel_GetShaderiv = NULL;
selGetInfoLog sel_GetShaderInfoLog = NULL;

struct sel_Proc_t {
    void** func;
    const char* names[3];
};

static void* s_get_proc(const char* name);
static char s_load_gl(void);
static void s_setup_gl(void);
static void s_close_gl(void);

static int l_minigl_setup(lua_State* L) {
    if (s_load_gl()) {
        s_setup_gl();
        s_close_gl();
        return 0;
    }
    return luaL_error(L, "[selene] failed to setup minigl");
}

static MODULE_FUNCTION(minigl, get_string) {
    INIT_ARG();
    CHECK_INTEGER(flag);
    PUSH_STRING((const char*)CALL_GL(GetString, flag));
    return 1;
}

static MODULE_FUNCTION(minigl, clear) {
    INIT_ARG();
    CHECK_INTEGER(flags);
    CALL_GL(Clear, flags);
    return 0;
}

static MODULE_FUNCTION(minigl, clear_color) {
    INIT_ARG();
    CHECK_NUMBER(float, r);
    CHECK_NUMBER(float, g);
    CHECK_NUMBER(float, b);
    CHECK_NUMBER(float, a);
    CALL_GL(ClearColor, r, g, b, a);
    return 0;
}

static MODULE_FUNCTION(minigl, enable) {
    INIT_ARG();
    CHECK_INTEGER(enable);
    CALL_GL(Enable, enable);
    return 0;
}

static MODULE_FUNCTION(minigl, disable) {
    INIT_ARG();
    CHECK_INTEGER(disable);
    CALL_GL(Disable, disable);
    return 0;
}

static MODULE_FUNCTION(minigl, blend_func) {
    INIT_ARG();
    CHECK_INTEGER(sfactor);
    CHECK_INTEGER(dfactor);
    CALL_GL(BlendFunc, sfactor, dfactor);
    return 0;
}

static MODULE_FUNCTION(minigl, blend_equation) {
    INIT_ARG();
    CHECK_INTEGER(eq);
    CALL_GL(BlendEquation, eq);
    return 0;
}

static MODULE_FUNCTION(minigl, draw_arrays) {
    INIT_ARG();
    CHECK_INTEGER(mode);
    CHECK_INTEGER(start);
    CHECK_INTEGER(count);
    CALL_GL(DrawArrays, mode, start, count);
    return 0;
}

static MODULE_FUNCTION(minigl, draw_elements) {
    INIT_ARG();
    CHECK_INTEGER(mode);
    CHECK_INTEGER(count);
    CHECK_INTEGER(type);
    CHECK_INTEGER(indices);
    CALL_GL(DrawElements, mode, count, type, (void*)indices);
    return 0;
}

/* Vertex Array */

static MODULE_FUNCTION(minigl, gen_vertex_arrays) {
    #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        CALL_GL(GenVertexArrays, 1, &t);
        lua_pushinteger(L, t);
    }
    return n;
    #else
    return 0;
    #endif
}

static MODULE_FUNCTION(minigl, bind_vertex_array) {
    #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    INIT_ARG();
    OPT_INTEGER(vao, 0);
    CALL_GL(BindVertexArray, vao);
    #endif
    return 0;
}

static MODULE_FUNCTION(minigl, delete_vertex_arrays) {
    #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(vao);
        CALL_GL(DeleteVertexArrays, 1, &vao);
    }
    #endif
    return 0;
}

static MODULE_FUNCTION(minigl, enable_vertex_attrib_array) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    CALL_GL(EnableVertexAttribArray, attrib);
    return 0;
}

static MODULE_FUNCTION(minigl, disable_vertex_attrib_array) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    CALL_GL(DisableVertexAttribArray, attrib);
    return 0;
}

static MODULE_FUNCTION(minigl, vertex_attrib_pointer) {
    INIT_ARG();
    CHECK_INTEGER(attrib);
    CHECK_INTEGER(size);
    CHECK_INTEGER(type);
    CHECK_BOOLEAN(normalized);
    CHECK_INTEGER(stride);
    CHECK_INTEGER(offset);
    CALL_GL(VertexAttribPointer, attrib, size, type, normalized, stride, (void*)offset);
    return 0;
}

/* Buffer */

static MODULE_FUNCTION(minigl, gen_buffers) {
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        glObject t;
        CALL_GL(GenBuffers, 1, &t);
        lua_pushinteger(L, t);
    }
    return n;
}

static MODULE_FUNCTION(minigl, bind_buffer) {
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(buf, 0);
    CALL_GL(BindBuffer, target, buf);
    return 0;
}

static MODULE_FUNCTION(minigl, delete_buffers) {
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(b);
        CALL_GL(DeleteBuffers, 1, &b);
    }
    return 0;
}

static MODULE_FUNCTION(minigl, buffer_data) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(size);
    const char* data = lua_touserdata(L, arg++);
    CHECK_INTEGER(usage);
    CALL_GL(BufferData, target, size, data, usage);
    return 0;
}

static MODULE_FUNCTION(minigl, buffer_subdata) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(start);
    CHECK_INTEGER(size);
    CHECK_LUDATA(const void*, data);
    CALL_GL(BufferSubData, target, start, size, data);
    return 0;
}

/* Texture */

static MODULE_FUNCTION(minigl, gen_textures) {
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int t;
        CALL_GL(GenTextures, 1, &t);
        lua_pushinteger(L, t);
    }
    return n;
}

static MODULE_FUNCTION(minigl, bind_texture) {
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(tex, 0);
    CALL_GL(BindTexture, target, tex);
    return 0;
}

static MODULE_FUNCTION(minigl, delete_textures) {
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(tex);
        CALL_GL(DeleteTextures, 1, &tex);
    }
    return 0;
}

static MODULE_FUNCTION(minigl, tex_image2d) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(level);
    CHECK_INTEGER(internal);
    CHECK_INTEGER(w);
    CHECK_INTEGER(h);
    CHECK_INTEGER(border);
    CHECK_INTEGER(format);
    CHECK_INTEGER(type);
    const void* data = lua_touserdata(L, arg);
    CALL_GL(TexImage2D, target, level, internal, w, h, border, format, type, data);
    return 0;
}

static MODULE_FUNCTION(minigl, tex_parameteri) {
    INIT_ARG();
    CHECK_INTEGER(tex);
    CHECK_INTEGER(pname);
    CHECK_INTEGER(param);
    CALL_GL(TexParameteri, tex, pname, param);
    return 0;
}

/* Framebuffer */

static MODULE_FUNCTION(minigl, gen_framebuffers) {
    INIT_ARG();
    CHECK_INTEGER(n);
    for (int i = 0; i < n; i++) {
        int f;
        CALL_GL(GenFramebuffers, 1, &f);
        lua_pushinteger(L, f);
    }
    return n;
}

static MODULE_FUNCTION(minigl, bind_framebuffer) {
    INIT_ARG();
    CHECK_INTEGER(target);
    OPT_INTEGER(fbo, 0);
    CALL_GL(BindFramebuffer, target, fbo);
    return 0;
}


static MODULE_FUNCTION(minigl, delete_framebuffers) {
    INIT_ARG();
    for (int i = 0; i < lua_gettop(L); i++) {
        CHECK_INTEGER(fbo);
        CALL_GL(DeleteFramebuffers, 1, &fbo);
    }
    return 0;
}

static MODULE_FUNCTION(minigl, framebuffer_texture2d) {
    INIT_ARG();
    CHECK_INTEGER(target);
    CHECK_INTEGER(attachment);
    CHECK_INTEGER(textarget);
    CHECK_INTEGER(texture);
    CHECK_INTEGER(level);
    CALL_GL(FramebufferTexture2D, target, attachment, textarget, texture, level);
    return 0;
}

/* Program */

static MODULE_FUNCTION(minigl, create_program) {
    int p = CALL_GL(CreateProgram);
    PUSH_INTEGER(p);
    return 1;
}

static MODULE_FUNCTION(minigl, use_program) {
    INIT_ARG();
    OPT_INTEGER(p, 0);
    CALL_GL(UseProgram, p);
    return 0;
}

static MODULE_FUNCTION(minigl, delete_program) {
    INIT_ARG();
    CHECK_INTEGER(p);
    CALL_GL(DeleteProgram, p);
    return 0;
}

static MODULE_FUNCTION(minigl, attach_shader) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_INTEGER(shader);
    CALL_GL(AttachShader, prog, shader);
    return 0;
}

static MODULE_FUNCTION(minigl, link_program) {
    INIT_ARG();
    CHECK_INTEGER(p);
    CALL_GL(LinkProgram, p);
    return 0;
}

static MODULE_FUNCTION(minigl, get_programiv) {
    INIT_ARG();
    CHECK_INTEGER(s);
    CHECK_INTEGER(pname);
    int out;
    CALL_GL(GetProgramiv, s, pname, &out);
    PUSH_INTEGER(out);
    return 1;
}

static MODULE_FUNCTION(minigl, get_program_info_log) {
    INIT_ARG();
    CHECK_INTEGER(s);
    char log[1024];
    CALL_GL(GetProgramInfoLog, s, 1024, NULL, log);
    PUSH_STRING(log);
    return 1;
}

static MODULE_FUNCTION(minigl, get_uniform_location) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_STRING(name);
    int loc = CALL_GL(GetUniformLocation, prog, name);
    PUSH_INTEGER(loc);
    return 1;
}

static MODULE_FUNCTION(minigl, get_attrib_location) {
    INIT_ARG();
    CHECK_INTEGER(prog);
    CHECK_STRING(name);
    int loc = CALL_GL(GetAttribLocation, prog, name);
    PUSH_INTEGER(loc);
    return 1;
}

#define DEFINE_LUA_UNIFORM(type, suffix)\
static MODULE_FUNCTION(minigl, uniform1##suffix) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_NUMBER(type, v0);\
    CALL_GL(Uniform1##suffix, loc, v0);\
    return 0;\
}\
static MODULE_FUNCTION(minigl, uniform2##suffix) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_NUMBER(type, v0);\
    CHECK_NUMBER(type, v1);\
    CALL_GL(Uniform2##suffix, loc, v0, v1);\
    return 0;\
}\
static MODULE_FUNCTION(minigl, uniform3##suffix) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_NUMBER(type, v0);\
    CHECK_NUMBER(type, v1);\
    CHECK_NUMBER(type, v2);\
    CALL_GL(Uniform3##suffix, loc, v0, v1, v2);\
    return 0;\
}\
static MODULE_FUNCTION(minigl, uniform4##suffix) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_NUMBER(type, v0);\
    CHECK_NUMBER(type, v1);\
    CHECK_NUMBER(type, v2);\
    CHECK_NUMBER(type, v3);\
    CALL_GL(Uniform4##suffix, loc, v0, v1, v2, v3);\
    return 0;\
}\
static MODULE_FUNCTION(minigl, uniform1##suffix##v) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_INTEGER(count);\
    float* m = NULL;\
    int tp = lua_type(L, arg);\
    if (tp == LUA_TLIGHTUSERDATA) {\
        m = lua_touserdata(L, arg);\
    } else {\
        for (int i = 0; i < count; i++) {\
            aux[i] = luaL_checknumber(L, arg + i);\
        }\
        m = aux;\
    }\
    CALL_GL(Uniform1fv, loc, count, aux);\
    return 0;\
}\
static MODULE_FUNCTION(minigl, uniform2##suffix##v) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_INTEGER(count);\
    float* m = NULL;\
    int tp = lua_type(L, arg);\
    if (tp == LUA_TLIGHTUSERDATA) {\
        m = lua_touserdata(L, arg);\
    } else {\
        for (int i = 0; i < count * 2; i++) {\
            aux[i] = luaL_checknumber(L, arg + i);\
        }\
        m = aux;\
    }\
    CALL_GL(Uniform2fv, loc, count, aux);\
    return 0;\
}\
static MODULE_FUNCTION(minigl, uniform3##suffix##v) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_INTEGER(count);\
    float* m = NULL;\
    int tp = lua_type(L, arg);\
    if (tp == LUA_TLIGHTUSERDATA) {\
        m = lua_touserdata(L, arg);\
    } else {\
        for (int i = 0; i < count * 3; i++) {\
            aux[i] = luaL_checknumber(L, arg + i);\
        }\
        m = aux;\
    }\
    CALL_GL(Uniform3fv, loc, count, aux);\
    return 0;\
}\
static MODULE_FUNCTION(minigl, uniform4##suffix##v) {\
    INIT_ARG();\
    CHECK_INTEGER(loc);\
    CHECK_INTEGER(count);\
    float* m = NULL;\
    int tp = lua_type(L, arg);\
    if (tp == LUA_TLIGHTUSERDATA) {\
        m = lua_touserdata(L, arg);\
    } else {\
        for (int i = 0; i < count * 4; i++) {\
            aux[i] = luaL_checknumber(L, arg + i);\
        }\
        m = aux;\
    }\
    CALL_GL(Uniform4fv, loc, count, aux);\
    return 0;\
}

float aux[512];
DEFINE_LUA_UNIFORM(float, f)
DEFINE_LUA_UNIFORM(int, i)

static MODULE_FUNCTION(minigl, uniform_matrix4fv) {
    INIT_ARG();
    CHECK_INTEGER(loc);
    CHECK_INTEGER(count);
    CHECK_BOOLEAN(transpose);
    int args = lua_gettop(L);
    float* m = NULL;
    int type = lua_type(L, arg);
    if (type == LUA_TLIGHTUSERDATA || type == LUA_TUSERDATA) {
        m = lua_touserdata(L, arg);
    } else {
        for (int i = 0; i < count * 16; i++) {
            aux[i] = luaL_checknumber(L, arg + i);
        }
        m = aux;
    }
    CALL_GL(UniformMatrix4fv, loc, count, transpose, m);
    return 0;
}

/* Shader */

static MODULE_FUNCTION(minigl, create_shader) {
    INIT_ARG();
    CHECK_INTEGER(type);
    int s = CALL_GL(CreateShader, type);
    PUSH_INTEGER(s);
    return 1;
}

static MODULE_FUNCTION(minigl, delete_shader) {
    INIT_ARG();
    CHECK_INTEGER(s);
    CALL_GL(DeleteShader, s);
    return 0;
}

static MODULE_FUNCTION(minigl, shader_source) {
    INIT_ARG();
    CHECK_INTEGER(s);
    CHECK_STRING(src);
    CALL_GL(ShaderSource, s, 1, &src, NULL);
    return 0;
}

static MODULE_FUNCTION(minigl, compile_shader) {
    INIT_ARG();
    CHECK_INTEGER(s);
    CALL_GL(CompileShader, s);
    return 0;
}

static MODULE_FUNCTION(minigl, get_shaderiv) {
    INIT_ARG();
    CHECK_INTEGER(s);
    CHECK_INTEGER(pname);
    int out;
    CALL_GL(GetShaderiv, s, pname, &out);
    PUSH_INTEGER(out);
    return 1;
}

static MODULE_FUNCTION(minigl, get_shader_info_log) {
    INIT_ARG();
    CHECK_INTEGER(s);
    char log[1024];
    CALL_GL(GetShaderInfoLog, s, 1024, NULL, log);
    PUSH_STRING(log);
    return 1;
}

BEGIN_ENUM(minigl)
    ENUM_FIELD(ZERO, SELGL_),
    ENUM_FIELD(ONE, SELGL_),

    ENUM_FIELD(POINTS, SELGL_),
    ENUM_FIELD(LINES, SELGL_),
    ENUM_FIELD(LINE_LOOP, SELGL_),
    ENUM_FIELD(LINE_STRIP, SELGL_),
    ENUM_FIELD(TRIANGLES, SELGL_),
    ENUM_FIELD(TRIANGLE_STRIP, SELGL_),
    ENUM_FIELD(TRIANGLE_FAN, SELGL_),

    ENUM_FIELD(NEVER, SELGL_),
    ENUM_FIELD(LESS, SELGL_),
    ENUM_FIELD(EQUAL, SELGL_),
    ENUM_FIELD(LEQUAL, SELGL_),
    ENUM_FIELD(GREATER, SELGL_),
    ENUM_FIELD(NOTEQUAL, SELGL_),
    ENUM_FIELD(GEQUAL, SELGL_),
    ENUM_FIELD(ALWAYS, SELGL_),
    ENUM_FIELD(ZERO, SELGL_),
    ENUM_FIELD(ONE, SELGL_),
    ENUM_FIELD(SRC_COLOR, SELGL_),
    ENUM_FIELD(ONE_MINUS_SRC_COLOR, SELGL_),
    ENUM_FIELD(SRC_ALPHA, SELGL_),
    ENUM_FIELD(ONE_MINUS_SRC_ALPHA, SELGL_),
    ENUM_FIELD(DST_ALPHA, SELGL_),
    ENUM_FIELD(ONE_MINUS_DST_ALPHA, SELGL_),
    ENUM_FIELD(DST_COLOR, SELGL_),
    ENUM_FIELD(ONE_MINUS_DST_COLOR, SELGL_),
    ENUM_FIELD(SRC_ALPHA_SATURATE, SELGL_),
    ENUM_FIELD(NONE, SELGL_),
    ENUM_FIELD(FRONT_LEFT, SELGL_),
    ENUM_FIELD(FRONT_RIGHT, SELGL_),
    ENUM_FIELD(BACK_LEFT, SELGL_),
    ENUM_FIELD(BACK_RIGHT, SELGL_),
    ENUM_FIELD(FRONT, SELGL_),
    ENUM_FIELD(BACK, SELGL_),
    ENUM_FIELD(LEFT, SELGL_),
    ENUM_FIELD(RIGHT, SELGL_),
    ENUM_FIELD(FRONT_AND_BACK, SELGL_),
    ENUM_FIELD(NO_ERROR, SELGL_),
    ENUM_FIELD(INVALID_ENUM, SELGL_),
    ENUM_FIELD(INVALID_VALUE, SELGL_),
    ENUM_FIELD(INVALID_OPERATION, SELGL_),
    ENUM_FIELD(OUT_OF_MEMORY, SELGL_),
    ENUM_FIELD(CW, SELGL_),
    ENUM_FIELD(CCW, SELGL_),
    ENUM_FIELD(POINT_SIZE, SELGL_),
    ENUM_FIELD(POINT_SIZE_RANGE, SELGL_),
    ENUM_FIELD(POINT_SIZE_GRANULARITY, SELGL_),
    ENUM_FIELD(LINE_SMOOTH, SELGL_),
    ENUM_FIELD(LINE_WIDTH, SELGL_),
    ENUM_FIELD(LINE_WIDTH_RANGE, SELGL_),
    ENUM_FIELD(LINE_WIDTH_GRANULARITY, SELGL_),
    ENUM_FIELD(POLYGON_MODE, SELGL_),
    ENUM_FIELD(POLYGON_SMOOTH, SELGL_),
    ENUM_FIELD(CULL_FACE, SELGL_),
    ENUM_FIELD(CULL_FACE_MODE, SELGL_),
    ENUM_FIELD(FRONT_FACE, SELGL_),
    ENUM_FIELD(DEPTH_RANGE, SELGL_),
    ENUM_FIELD(DEPTH_TEST, SELGL_),
    ENUM_FIELD(DEPTH_WRITEMASK, SELGL_),
    ENUM_FIELD(DEPTH_CLEAR_VALUE, SELGL_),
    ENUM_FIELD(DEPTH_FUNC, SELGL_),
    ENUM_FIELD(STENCIL_TEST, SELGL_),
    ENUM_FIELD(STENCIL_CLEAR_VALUE, SELGL_),
    ENUM_FIELD(STENCIL_FUNC, SELGL_),
    ENUM_FIELD(STENCIL_VALUE_MASK, SELGL_),
    ENUM_FIELD(STENCIL_FAIL, SELGL_),
    ENUM_FIELD(STENCIL_PASS_DEPTH_FAIL, SELGL_),
    ENUM_FIELD(STENCIL_PASS_DEPTH_PASS, SELGL_),
    ENUM_FIELD(STENCIL_REF, SELGL_),
    ENUM_FIELD(STENCIL_WRITEMASK, SELGL_),
    ENUM_FIELD(VIEWPORT, SELGL_),
    ENUM_FIELD(DITHER, SELGL_),
    ENUM_FIELD(BLEND_DST, SELGL_),
    ENUM_FIELD(BLEND_SRC, SELGL_),
    ENUM_FIELD(BLEND, SELGL_),

    ENUM_FIELD(VERSION, SELGL_),
    ENUM_FIELD(SHADING_LANGUAGE_VERSION, SELGL_),

    ENUM_FIELD(DEPTH_BUFFER_BIT, SELGL_),
    ENUM_FIELD(STENCIL_BUFFER_BIT, SELGL_),
    ENUM_FIELD(COLOR_BUFFER_BIT, SELGL_),

    ENUM_FIELD(TEXTURE_1D, SELGL_),
    ENUM_FIELD(TEXTURE_2D, SELGL_),
    ENUM_FIELD(TEXTURE_WIDTH, SELGL_),
    ENUM_FIELD(TEXTURE_HEIGHT, SELGL_),
    ENUM_FIELD(TEXTURE_BORDER_COLOR, SELGL_),
    ENUM_FIELD(DONT_CARE, SELGL_),
    ENUM_FIELD(FASTEST, SELGL_),
    ENUM_FIELD(NICEST, SELGL_),
    ENUM_FIELD(BYTE, SELGL_),
    ENUM_FIELD(UNSIGNED_BYTE, SELGL_),
    ENUM_FIELD(SHORT, SELGL_),
    ENUM_FIELD(UNSIGNED_SHORT, SELGL_),
    ENUM_FIELD(INT, SELGL_),
    ENUM_FIELD(UNSIGNED_INT, SELGL_),
    ENUM_FIELD(FLOAT, SELGL_),
    ENUM_FIELD(CLEAR, SELGL_),
    ENUM_FIELD(AND, SELGL_),
    ENUM_FIELD(AND_REVERSE, SELGL_),
    ENUM_FIELD(COPY, SELGL_),
    ENUM_FIELD(AND_INVERTED, SELGL_),
    ENUM_FIELD(NOOP, SELGL_),
    ENUM_FIELD(XOR, SELGL_),
    ENUM_FIELD(OR, SELGL_),
    ENUM_FIELD(NOR, SELGL_),
    ENUM_FIELD(EQUIV, SELGL_),
    ENUM_FIELD(INVERT, SELGL_),
    ENUM_FIELD(OR_REVERSE, SELGL_),
    ENUM_FIELD(COPY_INVERTED, SELGL_),
    ENUM_FIELD(OR_INVERTED, SELGL_),
    ENUM_FIELD(NAND, SELGL_),
    ENUM_FIELD(SET, SELGL_),
    ENUM_FIELD(TEXTURE, SELGL_),
    ENUM_FIELD(COLOR, SELGL_),
    ENUM_FIELD(DEPTH, SELGL_),
    ENUM_FIELD(STENCIL, SELGL_),
    ENUM_FIELD(STENCIL_INDEX, SELGL_),
    ENUM_FIELD(DEPTH_COMPONENT, SELGL_),
    ENUM_FIELD(RED, SELGL_),
    ENUM_FIELD(GREEN, SELGL_),
    ENUM_FIELD(BLUE, SELGL_),
    ENUM_FIELD(ALPHA, SELGL_),
    ENUM_FIELD(RGB, SELGL_),
    ENUM_FIELD(RGBA, SELGL_),
    ENUM_FIELD(POINT, SELGL_),
    ENUM_FIELD(LINE, SELGL_),
    ENUM_FIELD(FILL, SELGL_),
    ENUM_FIELD(KEEP, SELGL_),
    ENUM_FIELD(REPLACE, SELGL_),
    ENUM_FIELD(INCR, SELGL_),
    ENUM_FIELD(DECR, SELGL_),
    ENUM_FIELD(VENDOR, SELGL_),
    ENUM_FIELD(RENDERER, SELGL_),
    ENUM_FIELD(VERSION, SELGL_),
    ENUM_FIELD(EXTENSIONS, SELGL_),
    ENUM_FIELD(NEAREST, SELGL_),
    ENUM_FIELD(LINEAR, SELGL_),
    ENUM_FIELD(NEAREST_MIPMAP_NEAREST, SELGL_),
    ENUM_FIELD(LINEAR_MIPMAP_NEAREST, SELGL_),
    ENUM_FIELD(NEAREST_MIPMAP_LINEAR, SELGL_),
    ENUM_FIELD(LINEAR_MIPMAP_LINEAR, SELGL_),
    ENUM_FIELD(TEXTURE_MAG_FILTER, SELGL_),
    ENUM_FIELD(TEXTURE_MIN_FILTER, SELGL_),
    ENUM_FIELD(TEXTURE_WRAP_S, SELGL_),
    ENUM_FIELD(TEXTURE_WRAP_T, SELGL_),
    ENUM_FIELD(REPEAT, SELGL_),

    ENUM_FIELD(BGR, SELGL_),
    ENUM_FIELD(BGRA, SELGL_),
    ENUM_FIELD(CLAMP_TO_EDGE, SELGL_),
    ENUM_FIELD(CLAMP_TO_BORDER, SELGL_),
    ENUM_FIELD(BLEND_DST_RGB, SELGL_),
    ENUM_FIELD(BLEND_SRC_RGB, SELGL_),
    ENUM_FIELD(BLEND_DST_ALPHA, SELGL_),
    ENUM_FIELD(BLEND_SRC_ALPHA, SELGL_),
    ENUM_FIELD(POINT_FADE_THRESHOLD_SIZE, SELGL_),
    ENUM_FIELD(DEPTH_COMPONENT16, SELGL_),
    ENUM_FIELD(DEPTH_COMPONENT24, SELGL_),
    ENUM_FIELD(DEPTH_COMPONENT32, SELGL_),
    ENUM_FIELD(MIRRORED_REPEAT, SELGL_),

   ENUM_FIELD(BLEND_COLOR, SELGL_),
   ENUM_FIELD(BLEND_EQUATION, SELGL_),
   ENUM_FIELD(CONSTANT_COLOR, SELGL_),
   ENUM_FIELD(ONE_MINUS_CONSTANT_COLOR, SELGL_),
   ENUM_FIELD(CONSTANT_ALPHA, SELGL_),
   ENUM_FIELD(ONE_MINUS_CONSTANT_ALPHA, SELGL_),
   ENUM_FIELD(FUNC_ADD, SELGL_),
   ENUM_FIELD(FUNC_REVERSE_SUBTRACT, SELGL_),
   ENUM_FIELD(FUNC_SUBTRACT, SELGL_),
   ENUM_FIELD(MIN, SELGL_),
   ENUM_FIELD(MAX, SELGL_),

    ENUM_FIELD(ARRAY_BUFFER, SELGL_),
    ENUM_FIELD(ELEMENT_ARRAY_BUFFER, SELGL_),
    ENUM_FIELD(ARRAY_BUFFER_BINDING, SELGL_),
    ENUM_FIELD(ELEMENT_ARRAY_BUFFER_BINDING, SELGL_),

    ENUM_FIELD(STREAM_DRAW, SELGL_),
    ENUM_FIELD(STREAM_READ, SELGL_),
    ENUM_FIELD(STREAM_COPY, SELGL_),
    ENUM_FIELD(STATIC_DRAW, SELGL_),
    ENUM_FIELD(STATIC_READ, SELGL_),
    ENUM_FIELD(STATIC_COPY, SELGL_),
    ENUM_FIELD(DYNAMIC_DRAW, SELGL_),
    ENUM_FIELD(DYNAMIC_READ, SELGL_),
    ENUM_FIELD(DYNAMIC_COPY, SELGL_),

    ENUM_FIELD(FRAGMENT_SHADER, SELGL_),
    ENUM_FIELD(VERTEX_SHADER, SELGL_),
    ENUM_FIELD(COMPILE_STATUS, SELGL_),
    ENUM_FIELD(LINK_STATUS, SELGL_),
END_ENUM();

SELENE_API int luaopen_minigl(lua_State* L) {
    luaL_Reg reg[] = {
        REG_FIELD(minigl, setup),
        /* Draw */
        REG_FIELD(minigl, get_string),
        REG_FIELD(minigl, clear),
        REG_FIELD(minigl, clear_color),
        REG_FIELD(minigl, enable),
        REG_FIELD(minigl, disable),
        REG_FIELD(minigl, blend_func),
        REG_FIELD(minigl, blend_equation),
        REG_FIELD(minigl, draw_arrays),
        REG_FIELD(minigl, draw_elements),
        /* Vertex Array */
        REG_FIELD(minigl, gen_vertex_arrays),
        REG_FIELD(minigl, bind_vertex_array),
        REG_FIELD(minigl, delete_vertex_arrays),
        REG_FIELD(minigl, enable_vertex_attrib_array),
        REG_FIELD(minigl, disable_vertex_attrib_array),
        REG_FIELD(minigl, vertex_attrib_pointer),
        /* Buffer */
        REG_FIELD(minigl, gen_buffers),
        REG_FIELD(minigl, bind_buffer),
        REG_FIELD(minigl, delete_buffers),
        REG_FIELD(minigl, buffer_data),
        REG_FIELD(minigl, buffer_subdata),
        /* Texture */
        REG_FIELD(minigl, gen_textures),
        REG_FIELD(minigl, bind_texture),
        REG_FIELD(minigl, delete_textures),
        REG_FIELD(minigl, tex_parameteri),
        REG_FIELD(minigl, tex_image2d),
        /* Framebuffer */
        REG_FIELD(minigl, gen_framebuffers),
        REG_FIELD(minigl, bind_framebuffer),
        REG_FIELD(minigl, delete_framebuffers),
        REG_FIELD(minigl, framebuffer_texture2d),
        /* Program */
        REG_FIELD(minigl, create_program),
        REG_FIELD(minigl, delete_program),
        REG_FIELD(minigl, use_program),
        REG_FIELD(minigl, link_program),
        REG_FIELD(minigl, attach_shader),
        REG_FIELD(minigl, get_programiv),
        REG_FIELD(minigl, get_program_info_log),
        REG_FIELD(minigl, get_uniform_location),
        REG_FIELD(minigl, get_attrib_location),
        /* Shader */
        REG_FIELD(minigl, create_shader),
        REG_FIELD(minigl, delete_shader),
        REG_FIELD(minigl, shader_source),
        REG_FIELD(minigl, compile_shader),
        REG_FIELD(minigl, get_shaderiv),
        REG_FIELD(minigl, get_shader_info_log),
        REG_FIELD(minigl, uniform1f),
        REG_FIELD(minigl, uniform2f),
        REG_FIELD(minigl, uniform3f),
        REG_FIELD(minigl, uniform4f),
        REG_FIELD(minigl, uniform1fv),
        REG_FIELD(minigl, uniform2fv),
        REG_FIELD(minigl, uniform3fv),
        REG_FIELD(minigl, uniform4fv),
        REG_FIELD(minigl, uniform_matrix4fv),
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    LOAD_ENUM(minigl);
    return 1;
}

#if !defined(__APPLE__) && !defined(__HAIKU__)
void* (*s_proc_address)(const char*);
#endif

struct sel_Proc_t _procs[] = {
    FUNC_LOADER_FIELD(sel_Clear, glClear),
    FUNC_LOADER_FIELD(sel_ClearColor, glClearColor),
    FUNC_LOADER_FIELD(sel_Enable, glEnable),
    FUNC_LOADER_FIELD(sel_Disable, glDisable),
    FUNC_LOADER_FIELD(sel_BlendFunc, glBlendFunc),
    FUNC_LOADER_FIELD(sel_BlendEquation, glBlendEquation),
    FUNC_LOADER_FIELD(sel_DrawArrays, glDrawArrays),
    FUNC_LOADER_FIELD(sel_DrawElements, glDrawElements),
    FUNC_LOADER_FIELD(sel_GetUniformLocation, glGetUniformLocation),
    FUNC_LOADER_FIELD(sel_GetAttribLocation, glGetAttribLocation),
    UNIFORM_PROC(f),
    UNIFORM_PROC(i),
    FUNC_LOADER_FIELD(sel_UniformMatrix4fv, glUniformMatrix4fv),
    /* Vertex Arrays */
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    FUNC_LOADER_FIELD(sel_GenVertexArrays, glGenVertexArrays),
    FUNC_LOADER_FIELD(sel_BindVertexArray, glBindVertexArray),
    FUNC_LOADER_FIELD(sel_DeleteVertexArrays, glDeleteVertexArrays),
#endif
    FUNC_LOADER_FIELD(sel_EnableVertexAttribArray, glEnableVertexAttribArray),
    FUNC_LOADER_FIELD(sel_DisableVertexAttribArray, glDisableVertexAttribArray),
    FUNC_LOADER_FIELD(sel_VertexAttribPointer, glVertexAttribPointer),
    /* Buffers */
    FUNC_LOADER_FIELD(sel_GenBuffers, glGenBuffers),
    FUNC_LOADER_FIELD(sel_BindBuffer, glBindBuffer),
    FUNC_LOADER_FIELD(sel_DeleteBuffers, glDeleteBuffers),
    FUNC_LOADER_FIELD(sel_BufferData, glBufferData),
    FUNC_LOADER_FIELD(sel_BufferSubData, glBufferSubData),
    /* Textures */
    FUNC_LOADER_FIELD(sel_GenTextures, glGenTextures),
    FUNC_LOADER_FIELD(sel_BindTexture, glBindTexture),
    FUNC_LOADER_FIELD(sel_DeleteTextures, glDeleteTextures),
    FUNC_LOADER_FIELD(sel_TexImage2D, glTexImage2D),
    FUNC_LOADER_FIELD(sel_TexParameteri, glTexParameteri),
    FUNC_LOADER_FIELD(sel_TexParameterf, glTexParameterf),
    /* Framebuffer */
    FUNC_LOADER_FIELD(sel_GenFramebuffers, glGenFramebuffers),
    FUNC_LOADER_FIELD(sel_BindFramebuffer, glBindFramebuffer),
    FUNC_LOADER_FIELD(sel_DeleteFramebuffers, glDeleteFramebuffers),
    /* Program */
    FUNC_LOADER_FIELD(sel_CreateProgram, glCreateProgram),
    FUNC_LOADER_FIELD(sel_UseProgram, glUseProgram),
    FUNC_LOADER_FIELD(sel_DeleteProgram, glDeleteProgram),
    FUNC_LOADER_FIELD(sel_LinkProgram, glLinkProgram),
    FUNC_LOADER_FIELD(sel_AttachShader, glAttachShader),
    FUNC_LOADER_FIELD(sel_GetProgramiv, glGetProgramiv),
    FUNC_LOADER_FIELD(sel_GetProgramInfoLog, glGetProgramInfoLog),
    /* Shader */
    FUNC_LOADER_FIELD(sel_CreateShader, glCreateShader),
    FUNC_LOADER_FIELD(sel_DeleteShader, glDeleteShader),
    FUNC_LOADER_FIELD(sel_ShaderSource, glShaderSource),
    FUNC_LOADER_FIELD(sel_CompileShader, glCompileShader),
    FUNC_LOADER_FIELD(sel_GetShaderiv, glGetShaderiv),
    FUNC_LOADER_FIELD(sel_GetShaderInfoLog, glGetShaderInfoLog),
    {NULL, {NULL}}
};

char s_load_gl(void) {
#if defined(_WIN32)
    s_glsym = LoadLibraryW(L"opengl32.dll");
    if (s_glsym == NULL) {
        LPVOID lpMsgBuf;
        DWORD err = GetLastError();
        int s = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        char* str = (char*)lpMsgBuf;
        fprintf(stderr, "failed to load opengl32.dll: (%d) %s\n", err, str);
        LocalFree(lpMsgBuf);
        return 0;
    }
    s_proc_address = (void*(*)(const char*))GetProcAddress(s_glsym, "wglGetProcAddress");
    fprintf(stdout, "loaded OpenGL32.dll\n");
    return 1;
#else
#if defined(USE_GL_LIB)
    const char* names[] = {NULL};
    return 1;
#elif defined(__APPLE__)
    const char *names[] = {
        "../Frameworks/OpenGL.framework/OpenGL",
        "/Library/Frameworks/OpenGL.framework/Opengl",
        "/System/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL",
        NULL,
    };
#else
    const char *names[] = {
        "libGL.so.1",
        "libGL.so",
        NULL,
    };
#endif
    int index;
    for (index = 0; names[index] != NULL; ++index) {
        s_glsym = dlopen(names[index], RTLD_LAZY | RTLD_GLOBAL);
        if (s_glsym != NULL) {
#if defined(__APPLE__) || defined(__HAIKU__)
            return 1;
#else
            s_proc_address = (void*(*)(const char*))dlsym(s_glsym, "glXGetProcAddress");
            return s_proc_address != NULL;
#endif
            break;
        }
    }
#endif
    return 0;
}

void s_setup_gl() {
#if !defined(USE_GL_LIB)
    sel_GetString = s_get_proc("glGetString");
#else
    sel_GetString = (selGetString)glGetString;
#endif
    const char *version = (const char*)sel_GetString(SELGL_VERSION);
    const char *glsl = (const char*)sel_GetString(SELGL_SHADING_LANGUAGE_VERSION);
    if (version == NULL) {
        fprintf(stderr, "failed to get OpenGL version\n");
    }
    const char *prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        NULL,
    };

    fprintf(stderr, "OpenGL: %s\n", version);
    fprintf(stderr, "OpenGL shading language: %s\n", glsl);
#if defined(USE_GL_LIB)
    return;
#endif
    for (int i = 0; _procs[i].func != NULL; i++) {
        void* proc = s_get_proc(_procs[i].names[0]);
        // fprintf(stdout, "loading: %s, %p\n", _procs[i].names[0], proc);
        *(_procs[i].func) = proc;
    }
}

void s_close_gl(void) {
#if defined(USE_GL_LIB)
    return;
#endif
    if (s_glsym != NULL) {
#if defined(_WIN32)
        FreeLibrary(s_glsym);
#else
        dlclose(s_glsym);
#endif
        s_glsym = NULL;
    }
}

void* s_get_proc(const char *name) {
    void *sym = NULL;
    if (s_glsym == NULL) return sym;
#if !defined(__APPLE__) && !defined(__HAIKU__)
    if (s_proc_address != NULL) {
        sym = s_proc_address(name);
    }
#endif
    if (sym == NULL) {
#if defined(_WIN32) || defined(__CYGWIN__)
        sym = (void*)GetProcAddress(s_glsym, name);
#else
        sym = (void*)dlsym(s_glsym, name);
#endif
    }
    return sym;
}