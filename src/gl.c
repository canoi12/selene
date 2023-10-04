#include "selene.h"

#include "stb_image.h"
#include "stb_truetype.h"
#include "linmath.h"

#define TEXTURE_META "Texture"
#define FRAMEBUFFER_META "Framebuffer"

#define SHADER_META "Shader"
#define PROGRAM_META "Program"

#define VERTEX_ARRAY_META "VertexArray"
#define BUFFER_META "Buffer"

typedef unsigned int Texture;
typedef unsigned int Framebuffer;

typedef unsigned int Shader;
typedef unsigned int Program;

typedef unsigned int VertexArray;
typedef unsigned int Buffer;

struct GLInfo {
    Uint8 major, minor;
    Uint16 glsl;
    Uint16 es;
};

static struct GLInfo s_gl_info;
static Data* s_aux_data;

static int l_gl_LoadGlad(lua_State* L) {
#if !defined(__EMSCRIPTEN__)
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "failed to initialize OpenGL context\n");
        exit(EXIT_FAILURE);
    }
#endif
    return 0;
}

static int l_gl_PrintVersion(lua_State* L) {
    const Uint8* version = glGetString(GL_VERSION);
    const Uint8* glsl = glGetString(GL_SHADING_LANGUAGE_VERSION);
    fprintf(stderr, "%s // %s\n", version, glsl);
    const char *prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        NULL,
    };
    struct GLInfo *info = &s_gl_info;
    info->es = 0;
    char* ver = (char*)version;
    for (Uint32 i = 0; prefixes[i] != NULL; i++) {
        if (strncmp(ver, prefixes[i], strlen(prefixes[i])) == 0) {
            ver += strlen(prefixes[i]);
            info->es = 1;
            break;
        }
    }
    info->major = ver[0] - '0';
    info->minor = ver[2] - '0';
    if (info->es) {
        info->glsl = 100;
        if (info->major == 3) {
            info->glsl = info->major * 100 + info->minor * 10;
        }
    } else {
        float fglsl = atof((const char*)glsl);
        info->glsl = 100 * fglsl;
    }

    fprintf(stderr, "GL: { ver: %d.%d, glsl: %d, es: %s }\n", info->major, info->minor, info->glsl, info->es ? "true" : "false");
    return 0;
}

// Core
static int l_gl_get_viewport(lua_State* L) {
    int view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    lua_pushinteger(L, view[0]);
    lua_pushinteger(L, view[1]);
    lua_pushinteger(L, view[2]);
    lua_pushinteger(L, view[3]);
    return 4;
}

static int l_gl_Viewport(lua_State* L) {
    int view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        view[i] = luaL_checkinteger(L, 1+i);
    }
    glViewport(view[0], view[1], view[2], view[3]);
    return 0;
}

static int l_gl_ClearStencil(lua_State* L) {
    int s = luaL_checkinteger(L, 1);
    glClearStencil(s);
    return 0;
}

static int l_gl_ClearDepth(lua_State *L ) {
#if !defined(__EMSCRIPTEN__)
    glClearDepth(luaL_optnumber(L, 1, 1.0f));
#else
    glClearDepthf(luaL_optnumber(L, 1, 1.0f));
#endif
    return 0;
}

static int l_gl_ClearColor(lua_State* L) {
    float color[4] = { 0.f, 0.f, 0.f, 1.f };
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++)
        color[i] = lua_tonumber(L, i+1);
    glClearColor(color[0], color[1], color[2], color[3]);
    return 0;
}

static int l_gl_Clear(lua_State* L) {
    GLenum flags = 0;
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        flags |= luaL_checkinteger(L, 1+i);
    }
    glClear(flags);
    return 0;
}

static int l_gl_Enable(lua_State* L) {
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        glEnable(luaL_checkinteger(L, 1+i));
    }
    return 0;
}

static int l_gl_Disable(lua_State* L) {
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        glDisable(luaL_checkinteger(L, 1+i));
    }
    return 0;
}

static int l_gl_viewport(lua_State* L) {
    int r[4] = {0, 0, 640, 380};
    for (int i = i; i < lua_gettop(L); i++) {
        r[i] = luaL_checkinteger(L, 1+i);
    }
    glViewport(r[0], r[1], r[2], r[3]);
    return 0;
}


static int l_gl_Scissor(lua_State* L) {
    int x, y, w, h;
    x = luaL_checkinteger(L, 1);
    y = luaL_checkinteger(L, 2);
    w = luaL_checkinteger(L, 3);
    h = luaL_checkinteger(L, 4);
    glScissor(x, y, w, h);
    return 0;
}

static int l_gl_BlendFunc(lua_State* L) {
    int sfn, dfn;
    sfn = luaL_optinteger(L, 1, GL_SRC_ALPHA);
    dfn = luaL_optinteger(L, 2, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(sfn, dfn);
    return 0;
}

static int l_gl_DrawArrays(lua_State* L) {
    Uint16 mode = luaL_checkinteger(L, 1);
    Uint32 start = luaL_checkinteger(L, 2);
    Uint32 count = luaL_checkinteger(L, 3);
    glDrawArrays(mode, start, count);
    return 0;
}

static int l_gl_DrawElements(lua_State* L) {
    Uint16 mode = luaL_checkinteger(L, 1);
    Uint32 start = luaL_checkinteger(L, 2);
    Uint32 count = luaL_checkinteger(L, 3);
    Uint16 _type = luaL_checkinteger(L, 4);
    glDrawElements(mode, start, _type, (const void*)start);
    return 0;
}

// Texture
static int l_gl_NewTexture(lua_State* L) {
    Texture* tex = lua_newuserdata(L, sizeof(*tex));
    luaL_setmetatable(L, "Texture");
    glGenTextures(1, tex);
    return 1;
}

static int l_Texture__gc(lua_State* L) {
    Texture* tex = luaL_checkudata(L, 1, "Texture");
    glDeleteTextures(1, tex);
    return 0;
}

static int l_gl_BindTexture(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    Texture* tex = luaL_testudata(L, 2, "Texture");
    if (tex) glBindTexture(target, *tex);
    else glBindTexture(target, 0);
    return 0;
}

static int l_gl_TexImage2D(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    int internal = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int format = luaL_checkinteger(L, 5);
    int type_ = luaL_checkinteger(L, 6);
    void* data = NULL;
    if (lua_type(L, 7) == LUA_TUSERDATA) {
        Data* dt = luaL_checkudata(L, 7, "Data");
        data = dt->data;
    }
    glTexImage2D(target, 0, internal, width, height, 0, format, type_, data);
    return 0;
}

static int l_gl_TexSubImage2D(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    int xoffset = luaL_checkinteger(L, 2);
    int yoffset = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int format = luaL_checkinteger(L, 6);
    int type_ = luaL_checkinteger(L, 7);
    void* data = NULL;
    if (lua_type(L, 8) == LUA_TUSERDATA) {
        Data* dt = luaL_checkudata(L, 8, "Data");
        data = dt->data;
    }
    glTexSubImage2D(target, 0, xoffset, yoffset, width, height, format, type_, data);
    return 0;
}

static int l_gl_TexParameteri(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    int pname = luaL_checkinteger(L, 2);
    int param = luaL_checkinteger(L, 3);
    glTexParameteri(target, pname, param);
    return 0;
}

static int l_Texture_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"__gc", l_Texture__gc},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Texture");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

// Framebuffer
static int default_framebuffer;
static int l_gl_NewFramebuffer(lua_State* L) {
    Framebuffer* buffer = lua_newuserdata(L, sizeof(*buffer));
    luaL_setmetatable(L, "Framebuffer");
    glGenFramebuffers(1, buffer);
    return 1;
}

static int l_Framebuffer__gc(lua_State* L) {
    Framebuffer* fbo = luaL_checkudata(L, 1, "Framebuffer");
    glDeleteFramebuffers(1, fbo);
    return 0;
}

static int l_gl_BindFramebuffer(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    Framebuffer* buffer = luaL_testudata(L, 2, "Framebuffer");
    if (buffer) glBindFramebuffer(target, *buffer);
    else glBindFramebuffer(target, 0);
    return 0;
}

static int l_gl_AttachTexture2D(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    int attachment = luaL_checkinteger(L, 2);
    int tex_target = luaL_checkinteger(L, 3);
    Texture* tex = luaL_checkudata(L, 4, "Texture");
    glFramebufferTexture2D(target, attachment, tex_target, *tex, 0);
    return 0;
}

static int l_Framebuffer_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"__gc", l_Framebuffer__gc},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Framebuffer");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

// Vertex Array

static int l_gl_NewVertexArray(lua_State* L) {
    VertexArray* vao = lua_newuserdata(L, sizeof(*vao));
    luaL_setmetatable(L, "VertexArray");
#if !defined(__EMSCRIPTEN__)
    glGenVertexArrays(1, vao);
#endif
    return 1;
}

static int l_VertexArray__gc(lua_State* L) {
    VertexArray* vao = luaL_checkudata(L, 1, "VertexArray");
#if !defined(__EMSCRIPTEN__)
    glDeleteVertexArrays(1, vao);
#endif
    return 0;
}

static int l_gl_BindVertexArray(lua_State* L) {
    VertexArray* vao = luaL_testudata(L, 1, "VertexArray");
#if !defined(__EMSCRIPTEN__)
    if (vao) glBindVertexArray(*vao);
    else glBindVertexArray(0);
#endif
    return 0;
}

static int l_gl_EnableVertexAttribArray(lua_State* L) {
    int attrib = luaL_checkinteger(L, 1);
    glEnableVertexAttribArray(attrib);
    return 0;
}

static int l_gl_DisableVertexAttribArray(lua_State* L) {
    int attrib = luaL_checkinteger(L, 1);
    glDisableVertexAttribArray(attrib);
    return 0;
}

static int l_gl_VertexAttribPointer(lua_State* L) {
    int attrib = luaL_checkinteger(L, 1);
    int size = luaL_checkinteger(L, 2);
    int type = luaL_checkinteger(L, 3);
    int normalized = lua_toboolean(L, 4);
    int stride = luaL_checkinteger(L, 5);
    int offset = luaL_checkinteger(L, 6);
    glVertexAttribPointer(attrib, size, type, normalized, stride, (void*)offset);
    return 0;
}

static int l_VertexArray_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"__gc", l_VertexArray__gc},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "VertexArray");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

// Buffer
static int l_gl_NewBuffer(lua_State* L) {
    Buffer* buffer = lua_newuserdata(L, sizeof(*buffer));
    luaL_setmetatable(L, "Buffer");
    glGenBuffers(1, buffer);
    return 1;
}

static int l_Buffer__gc(lua_State* L) {
    Buffer* b = luaL_checkudata(L, 1, BUFFER_META);
    glDeleteBuffers(1, b);
    return 0;
}

static int l_gl_BindBuffer(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    Buffer* b = luaL_testudata(L, 2, BUFFER_META);
    if (b) glBindBuffer(target, *b);
    else glBindBuffer(target, 0);
    return 0;
}

static int l_gl_BufferData(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    int size = luaL_checkinteger(L, 2);
    int usage = luaL_checkinteger(L, 3);
    Data* data = luaL_testudata(L, 4, "Data");
    if (data) glBufferData(target, size, data->data, usage);
    else glBufferData(target, size, NULL, usage);
    return 0;
}

static int l_gl_BufferSubData(lua_State* L) {
    int target = luaL_checkinteger(L, 1);
    int start = luaL_checkinteger(L, 2);
    int size = luaL_checkinteger(L, 3);
    Data* data = luaL_checkudata(L, 4, "Data");
    glBufferSubData(target, start, size, data->data);
    return 0;
}

static int l_Buffer_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"__gc", l_Buffer__gc},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Buffer");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

// Shader
static int l_gl_NewShader(lua_State* L) {
    int gl_enum = luaL_checkinteger(L, 1);
    Shader* s = lua_newuserdata(L, sizeof(*s));
    luaL_setmetatable(L, "Shader");
    *s = glCreateShader(gl_enum);
    return 1;
}

static int l_Shader__gc(lua_State* L) {
    Shader* s = luaL_checkudata(L, 1, "Shader");
    glDeleteShader(*s);
    *s = 0;
    return 0;
}

static int l_gl_ShaderSource(lua_State* L) {
    Shader* s = luaL_checkudata(L, 1, "Shader");
    const char* source = luaL_checkstring(L, 2);
    glShaderSource(*s, 1, &source, NULL);
    return 0;
}

static int l_gl_CompileShader(lua_State* L) {
    Shader* s = luaL_checkudata(L, 1, "Shader");
    glCompileShader(*s);
    int success = 0;
    glGetShaderiv(*s, GL_COMPILE_STATUS, &success);
    if (!success) {
        int len;
        glGetShaderiv(*s, GL_INFO_LOG_LENGTH, &len);
    #if defined(OS_WIN)
        if (len+1 > s_aux_data->size) {
            s_aux_data->size = (len+1)*2;
            s_aux_data->data = realloc(s_aux_data->data, s_aux_data->size);
        }
        char* log = s_aux_data->data;
    #else
        char log[len+1];
    #endif
        glGetShaderInfoLog(*s, len, NULL, log);
        log[len] = '\0';
        return luaL_error(L, "Failed to compile shader: %s\n", log);
    }
    return 0;
}

static int l_Shader_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"__gc", l_Shader__gc},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Shader");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

// Program
static int l_gl_NewProgram(lua_State* L) {
    Program* p = lua_newuserdata(L, sizeof(*p));
    luaL_setmetatable(L, "Program");
    *p = glCreateProgram();
    return 1;
}

static int l_Program__gc(lua_State* L) {
    Program* p = luaL_checkudata(L, 1, "Program");
    glDeleteProgram(*p);
    return 0;
}

static int l_gl_AttachShader(lua_State* L) {
    Program* p = luaL_checkudata(L, 1, "Program");
    int args = lua_gettop(L)-1;
    for (int i = 0; i < args; i++) {
        Shader* s = luaL_checkudata(L, 2+i, "Shader");
        glAttachShader(*p, *s);
    }
    return 0;
}

static int l_gl_LinkProgram(lua_State* L) {
    Program* p = luaL_checkudata(L, 1, "Program");
    glLinkProgram(*p);
    int success = 0;
    glGetProgramiv(*p, GL_LINK_STATUS, &success);
    if (!success) {
        int len;
        glGetProgramiv(*p, GL_INFO_LOG_LENGTH, &len);
    #if defined(OS_WIN)
        if (len+1 > s_aux_data->size) {
            s_aux_data->size = (len+1)*2;
            s_aux_data->data = realloc(s_aux_data->data, s_aux_data->size);
        }
        char* log = s_aux_data->data;
    #else
        char log[len+1];
    #endif
        glGetProgramInfoLog(*p, len, NULL, log);
        log[len] = '\0';
        return luaL_error(L, "Failed to link program: %s\n", log);
    }

    return 0;
}

static int l_gl_UseProgram(lua_State* L) {
    Program* p = luaL_testudata(L, 1, "Program");
    if (p) glUseProgram(*p);
    else glUseProgram(0);
    return 0;
}

static int l_gl_GetAttribLocation(lua_State* L) {
    Program* p = luaL_checkudata(L, 1, "Program");
    const char* name = luaL_checkstring(L, 2);
    int loc = glGetAttribLocation(*p, name);
    lua_pushinteger(L, loc);
    return 1;
}

static int l_gl_GetUniformLocation(lua_State* L) {
    Program* p = luaL_checkudata(L, 1, "Program");
    const char* name = luaL_checkstring(L, 2);
    int loc = glGetUniformLocation(*p, name);
    lua_pushinteger(L, loc);
    return 1;
}

static int l_gl_Uniform1fv(lua_State* L) {
    int location = (int)luaL_checkinteger(L, 1);
    int args = (int)lua_gettop(L) - 1;
#if defined(OS_WIN)
    int size = args * 4;
    if (size > s_aux_data->size) {
        s_aux_data->size = size * 2;
        s_aux_data->data = realloc(s_aux_data->data, size*2);
    }
    float* values = s_aux_data->data;
#else
    float values[args];
#endif
    for (int i = 0; i < args; i++) {
        values[i] = luaL_checknumber(L, 2+i);
    }
    glUniform1fv(location, args, values);
    return 0;
}

static int l_gl_Uniform2fv(lua_State* L) {
    int location = luaL_checkinteger(L, 1);
    int args = lua_gettop(L) - 2;
    int size = args * 4 * 2;
#if defined(OS_WIN)
    if (size > s_aux_data->size) {
        s_aux_data->size = size * 2;
        s_aux_data->data = realloc(s_aux_data->data, size*2);
    }
    float* values = s_aux_data->data;
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
            v[j++] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 2;
    }
    glUniform2fv(location, args, values);
    return 0;
}

static int l_gl_Uniform3fv(lua_State* L) {
    int location = luaL_checkinteger(L, 1);
    int args = lua_gettop(L) - 2;
    int size = args * 4 * 3;
#if defined(OS_WIN)
    if (size > s_aux_data->size) {
        s_aux_data->size = size * 2;
        s_aux_data->data = realloc(s_aux_data->data, size*2);
    }
    float* values = s_aux_data->data;
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
            v[j++] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 3;
    }
    glUniform3fv(location, args, values);
    return 0;
}

static int l_gl_Uniform4fv(lua_State* L) {
    int location = luaL_checkinteger(L, 1);
    int args = lua_gettop(L) - 2;
    int size = args * 4 * 4;
#if defined(OS_WIN)
    if (size > s_aux_data->size) {
        s_aux_data->size = size * 2;
        s_aux_data->data = realloc(s_aux_data->data, size*2);
    }
    float* values = s_aux_data->data;
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
            v[j++] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 4;
    }
    glUniform4fv(location, args, values);
    return 0;
}

static int l_gl_UniformMatrix4fv(lua_State* L) {
    int location = luaL_checkinteger(L, 1);
    int count = luaL_checkinteger(L, 2);
    int normalize = lua_toboolean(L, 3);
    mat4x4* m = luaL_checkudata(L, 4, "Mat4");
    glUniformMatrix4fv(location, count, normalize, **m);
    return 0;
}


static int l_Program_meta(lua_State* L) {
    luaL_Reg reg[] = {
        {"__gc", l_Program__gc},
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Program");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static int l_gl_gl(lua_State* L) {
    lua_newtable(L);
    return 1;
}

static struct {
    const char* name;
    GLenum value;
} l_gl_Enums[] = {
    // Clear
    {"COLOR_BUFFER_BIT", GL_COLOR_BUFFER_BIT},
    {"DEPTH_BUFFER_BIT", GL_DEPTH_BUFFER_BIT},
    {"STENCIL_BUFFER_BIT", GL_STENCIL_BUFFER_BIT},
    // Types
    {"BYTE", GL_BYTE},
    {"UNSIGNED_BYTE", GL_UNSIGNED_BYTE},
    {"SHORT", GL_SHORT},
    {"UNSIGNED_SHORT", GL_UNSIGNED_SHORT},
    {"INT", GL_INT},
    {"UNSIGNED_INT", GL_UNSIGNED_INT},
    {"FLOAT", GL_FLOAT},
#if !defined(__EMSCRIPTEN__)
    {"DOUBLE", GL_DOUBLE},
#endif
    // Enable
    {"DEPTH_TEST", GL_DEPTH_TEST},
    {"STENCIL_TEST", GL_STENCIL_TEST},
    {"SCISSOR_TEST", GL_SCISSOR_TEST},
    {"BLEND", GL_BLEND},
    {"CULL_FACE", GL_CULL_FACE},
    // Pixel
#if !defined(__EMSCRIPTEN__)
    {"RED", GL_RED},
    {"RG", GL_RG},
    {"BGR", GL_BGR},
    {"BGRA", GL_BGRA},
#endif
    {"RGB", GL_RGB},
    {"RGBA", GL_RGBA},
    {"DEPTH_COMPONENT", GL_DEPTH_COMPONENT},
    {"DEPTH_COMPONENT16", GL_DEPTH_COMPONENT16},
#if !defined(__EMSCRIPTEN__)
    {"DEPTH_COMPONENT24", GL_DEPTH_COMPONENT24},
    {"DEPTH_COMPONENT32", GL_DEPTH_COMPONENT32},
    {"DEPTH_COMPONENT32F", GL_DEPTH_COMPONENT32F},
    {"DEPTH32F_STENCIL8", GL_DEPTH32F_STENCIL8},
    {"DEPTH24_STENCIL8", GL_DEPTH24_STENCIL8},
#endif
    // Funcs
    {"ZERO", GL_ZERO},
    {"ONE", GL_ONE},
    {"SRC_COLOR", GL_SRC_COLOR},
    {"ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR},
    {"SRC_ALPHA", GL_SRC_ALPHA},
    {"ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA},
    {"DST_ALPHA", GL_DST_ALPHA},
    {"ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA},
    {"DST_COLOR", GL_DST_COLOR},
    {"ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR},
    {"SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE},
    {"CONSTANT_COLOR", GL_CONSTANT_COLOR},
    {"ONE_MINUS_CONSTANT_COLOR", GL_ONE_MINUS_CONSTANT_COLOR},
    {"CONSTANT_ALPHA", GL_CONSTANT_ALPHA},
    {"ONE_MINUS_CONSTANT_ALPHA", GL_ONE_MINUS_CONSTANT_ALPHA},
    // Draw mode
    {"POINTS", GL_POINTS},
    {"LINES", GL_LINES},
    {"TRIANGLES", GL_TRIANGLES},
    // Texture targers
    {"TEXTURE_2D", GL_TEXTURE_2D},
    {"TEXTURE_CUBE_MAP", GL_TEXTURE_CUBE_MAP},
    {"TEXTURE_CUBE_MAP_NEGATIVE_X", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
    {"TEXTURE_CUBE_MAP_POSITIVE_X", GL_TEXTURE_CUBE_MAP_POSITIVE_X},
    {"TEXTURE_CUBE_MAP_NEGATIVE_Y", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
    {"TEXTURE_CUBE_MAP_POSITIVE_Y", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
    {"TEXTURE_CUBE_MAP_NEGATIVE_Z", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
    {"TEXTURE_CUBE_MAP_POSITIVE_Z", GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
    // Texture params
    {"TEXTURE_MIN_FILTER", GL_TEXTURE_MIN_FILTER},
    {"TEXTURE_MAG_FILTER", GL_TEXTURE_MAG_FILTER},
    {"TEXTURE_WRAP_S", GL_TEXTURE_WRAP_S},
    {"TEXTURE_WRAP_T", GL_TEXTURE_WRAP_T},
    {"NEAREST", GL_NEAREST},
    {"LINEAR", GL_LINEAR},
    {"REPEAT", GL_NEAREST},
#if !defined(__EMSCRIPTEN__)
    {"CLAMP_TO_BORDER", GL_CLAMP_TO_BORDER},
    {"CLAMP_TO_EDGE", GL_CLAMP_TO_EDGE},
#endif
    // Framebuffer targets
    {"FRAMEBUFFER", GL_FRAMEBUFFER},
#if !defined(__EMSCRIPTEN__)
    {"DRAW_FRAMEBUFFER", GL_DRAW_FRAMEBUFFER},
    {"READ_FRAMEBUFFER", GL_READ_FRAMEBUFFER},
    {"DEPTH_STENCIL_ATTACHMENT", GL_DEPTH_STENCIL_ATTACHMENT},
#endif
    {"COLOR_ATTACHMENT0", GL_COLOR_ATTACHMENT0},
    {"DEPTH_ATTACHMENT", GL_DEPTH_ATTACHMENT},
    {"STENCIL_ATTACHMENT", GL_STENCIL_ATTACHMENT},
    // Buffer
    {"ARRAY_BUFFER", GL_ARRAY_BUFFER},
    {"ELEMENT_ARRAY_BUFFER", GL_ELEMENT_ARRAY_BUFFER},
#if !defined(__EMSCRIPTEN__)
    {"UNIFORM_BUFFER", GL_UNIFORM_BUFFER},
    {"DYNAMIC_READ", GL_DYNAMIC_READ},
    {"DYNAMIC_COPY", GL_DYNAMIC_COPY},
    {"STATIC_READ", GL_STATIC_READ},
    {"STATIC_COPY", GL_STATIC_COPY},
    {"STREAM_READ", GL_STREAM_READ},
    {"STREAM_COPY", GL_STREAM_COPY},
#endif
    {"DYNAMIC_DRAW", GL_DYNAMIC_DRAW},
    {"STATIC_DRAW", GL_STATIC_DRAW},
    {"STREAM_DRAW", GL_STREAM_DRAW},
    // Shader
    {"FRAGMENT_SHADER", GL_FRAGMENT_SHADER},
    {"VERTEX_SHADER", GL_VERTEX_SHADER},
#if !defined(__EMSCRIPTEN__)
    {"GEOMETRY_SHADER", GL_GEOMETRY_SHADER},
#endif
    {NULL, 0}
};

// luaopen
int seleneopen_gl(lua_State* L) {
    s_aux_data = lua_newuserdata(L, sizeof(Data));
    luaL_setmetatable(L, "Data");
    s_aux_data->offset = 0;
    s_aux_data->size = 40 * sizeof(float);
    s_aux_data->data = malloc(s_aux_data->size);
    lua_rawsetp(L, LUA_REGISTRYINDEX, s_aux_data);

    luaL_Reg reg[] = {
        {"LoadGlad", l_gl_LoadGlad},
        {"PrintVersion", l_gl_PrintVersion},
        // Core
        {"Viewport", l_gl_Viewport},
        {"Clear", l_gl_Clear},
        {"ClearColor", l_gl_ClearColor},
        {"ClearDepth", l_gl_ClearDepth},
        {"ClearStencil", l_gl_ClearStencil},
        {"Enable", l_gl_Enable},
        {"Disable", l_gl_Disable},
        {"Scissor", l_gl_Scissor},
        {"BlendFunc", l_gl_BlendFunc},
        {"DrawArrays", l_gl_DrawArrays},
        {"DrawElements", l_gl_DrawElements},
        // Texture
        {"NewTexture", l_gl_NewTexture},
        {"BindTexture", l_gl_BindTexture},
        {"TexImage2D", l_gl_TexImage2D},
        {"TexSubImage2D", l_gl_TexSubImage2D},
        {"TexParameteri", l_gl_TexParameteri},
        // Framebuffer
        {"NewFramebuffer", l_gl_NewFramebuffer},
        {"BindFramebuffer", l_gl_BindFramebuffer},
        {"AttachTexture2D", l_gl_AttachTexture2D},
        // Vertex
        {"NewVertexArray", l_gl_NewVertexArray},
        {"BindVertexArray", l_gl_BindVertexArray},
        {"EnableVertexAttribArray", l_gl_EnableVertexAttribArray},
        {"DisableVertexAttribArray", l_gl_DisableVertexAttribArray},
        {"VertexAttribPointer", l_gl_VertexAttribPointer},
        // Buffer
        {"NewBuffer", l_gl_NewBuffer},
        {"BindBuffer", l_gl_BindBuffer},
        {"BufferData", l_gl_BufferData},
        {"BufferSubData", l_gl_BufferSubData},
        // Shader
        {"NewShader", l_gl_NewShader},
        {"ShaderSource", l_gl_ShaderSource},
        {"CompileShader", l_gl_CompileShader},
        // Program
        {"NewProgram", l_gl_NewProgram},
        {"UseProgram", l_gl_UseProgram},
        {"AttachShader", l_gl_AttachShader},
        {"LinkProgram", l_gl_LinkProgram},
        {"GetAttribLocation", l_gl_GetAttribLocation},
        {"GetUniformLocation", l_gl_GetUniformLocation},
        {"Uniform1fv", l_gl_Uniform1fv},
        {"Uniform2fv", l_gl_Uniform2fv},
        {"Uniform3fv", l_gl_Uniform3fv},
        {"Uniform4fv", l_gl_Uniform4fv},
        {"UniformMatrix4fv", l_gl_UniformMatrix4fv},
        {NULL, NULL}
    };
    luaL_newlib(L, reg);

    for (int i = 0; l_gl_Enums[i].name != NULL; i++) {
        lua_pushinteger(L, l_gl_Enums[i].value);
        lua_setfield(L, -2, l_gl_Enums[i].name);
    }

    l_Texture_meta(L);
    lua_setfield(L, -2, "Texture");

    l_Framebuffer_meta(L);
    lua_setfield(L, -2, "Framebuffer");

    l_Shader_meta(L);
    lua_setfield(L, -2, "Shader");
    l_Program_meta(L);
    lua_setfield(L, -2, "Program");

    l_VertexArray_meta(L);
    lua_setfield(L, -2, "VertexArray");
    l_Buffer_meta(L);
    lua_setfield(L, -2, "Buffer");
    return 1;
}
