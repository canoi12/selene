#include "selene.h"

struct GLInfo {
    Uint8 major, minor;
    Uint16 glsl;
    Uint16 es;
};

static struct GLInfo s_gl_info;
#if defined(OS_WIN)
static Data* s_aux_data;
#endif

static BEGIN_FUNCTION(gl, LoadGlad)
#if !defined(__EMSCRIPTEN__)
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "failed to initialize OpenGL context\n");
        exit(EXIT_FAILURE);
    }
#endif
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, PrintVersion)
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
        float fglsl = (float)atof((const char*)glsl);
        info->glsl = (Uint16)(100 * fglsl);
    }

    fprintf(stderr, "GL: { ver: %d.%d, glsl: %d, es: %s }\n", info->major, info->minor, info->glsl, info->es ? "true" : "false");
END_FUNCTION(0)

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

static BEGIN_FUNCTION(gl, Viewport)
    int view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        view[i] = (int)luaL_checkinteger(L, 1+i);
    }
    glViewport(view[0], view[1], view[2], view[3]);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, ClearStencil)
    int s = (int)luaL_checkinteger(L, 1);
    glClearStencil(s);
END_FUNCTION(0)

static int l_gl_ClearDepth(lua_State *L ) {
#if !defined(__EMSCRIPTEN__)
    glClearDepth((float)luaL_optnumber(L, 1, 1.0f));
#else
    glClearDepthf((float)luaL_optnumber(L, 1, 1.0f));
#endif
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, ClearColor)
    float color[4] = { 0.f, 0.f, 0.f, 1.f };
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++)
        color[i] = (float)lua_tonumber(L, i+1);
    glClearColor(color[0], color[1], color[2], color[3]);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, Clear)
    GLenum flags = 0;
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        flags |= luaL_checkinteger(L, 1+i);
    }
    glClear(flags);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, Enable)
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        glEnable((GLenum)luaL_checkinteger(L, 1+i));
    }
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, Disable)
    int args = lua_gettop(L);
    for (int i = 0; i < args; i++) {
        glDisable((GLenum)luaL_checkinteger(L, 1+i));
    }
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, viewport)
    int r[4] = {0, 0, 640, 380};
    for (int i = i; i < lua_gettop(L); i++) {
        r[i] = (int)luaL_checkinteger(L, 1+i);
    }
    glViewport(r[0], r[1], r[2], r[3]);
END_FUNCTION(0)


static BEGIN_FUNCTION(gl, Scissor)
    int x, y, w, h;
    x = (int)luaL_checkinteger(L, 1);
    y = (int)luaL_checkinteger(L, 2);
    w = (int)luaL_checkinteger(L, 3);
    h = (int)luaL_checkinteger(L, 4);
    glScissor(x, y, w, h);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, BlendFunc)
    int sfn, dfn;
    sfn = (int)luaL_optinteger(L, 1, GL_SRC_ALPHA);
    dfn = (int)luaL_optinteger(L, 2, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(sfn, dfn);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, DrawArrays)
    Uint16 mode = (Uint16)luaL_checkinteger(L, 1);
    Uint32 start = (Uint32)luaL_checkinteger(L, 2);
    Uint32 count = (Uint32)luaL_checkinteger(L, 3);
    glDrawArrays(mode, start, count);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, DrawElements)
    Uint16 mode = (Uint16)luaL_checkinteger(L, 1);
    Uint32 start = (Uint32)luaL_checkinteger(L, 2);
    Uint32 count = (Uint32)luaL_checkinteger(L, 3);
    Uint16 _type = (Uint16)luaL_checkinteger(L, 4);
    glDrawElements(mode, start, _type, (void*)start);
END_FUNCTION(0)

/************************
 #                      #
 #       Texture        #
 #                      #
 ************************/
static BEGIN_FUNCTION(gl, NewTexture)
    NEW_UDATA(Texture, tex);
    glGenTextures(1, tex);
END_FUNCTION(1);

static BEGIN_META_FUNCTION(Texture, gc)
    glDeleteTextures(1, self);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, BindTexture)
    int target = (int)luaL_checkinteger(L, 1);
    Texture* tex = luaL_testudata(L, 2, "Texture");
    if (tex) glBindTexture(target, *tex);
    else glBindTexture(target, 0);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, TexImage2D)
    int target = (int)luaL_checkinteger(L, 1);
    int internal = (int)luaL_checkinteger(L, 2);
    int width = (int)luaL_checkinteger(L, 3);
    int height = (int)luaL_checkinteger(L, 4);
    int format = (int)luaL_checkinteger(L, 5);
    int type_ = (int)luaL_checkinteger(L, 6);
    void* data = NULL;
    if (lua_type(L, 7) == LUA_TUSERDATA) {
        Data* dt = luaL_checkudata(L, 7, "Data");
        data = dt->data;
    }
    glTexImage2D(target, 0, internal, width, height, 0, format, type_, data);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, TexSubImage2D)
    int target = (int)luaL_checkinteger(L, 1);
    int xoffset = (int)luaL_checkinteger(L, 2);
    int yoffset = (int)luaL_checkinteger(L, 3);
    int width = (int)luaL_checkinteger(L, 4);
    int height = (int)luaL_checkinteger(L, 5);
    int format = (int)luaL_checkinteger(L, 6);
    int type_ = (int)luaL_checkinteger(L, 7);
    void* data = NULL;
    if (lua_type(L, 8) == LUA_TUSERDATA) {
        Data* dt = luaL_checkudata(L, 8, "Data");
        data = dt->data;
    }
    glTexSubImage2D(target, 0, xoffset, yoffset, width, height, format, type_, data);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, TexParameteri)
    int target = (int)luaL_checkinteger(L, 1);
    int pname = (int)luaL_checkinteger(L, 2);
    int param = (int)luaL_checkinteger(L, 3);
    glTexParameteri(target, pname, param);
END_FUNCTION(0)

static BEGIN_META(Texture)
    BEGIN_REG(Texture)
        REG_META_FIELD(Texture, gc),
    END_REG()
    NEW_META(Texture);
END_META(1)

/************************
 #                      #
 #      Framebuffer     #
 #                      #
 ************************/
static BEGIN_FUNCTION(gl, NewFramebuffer)
    Framebuffer* buffer = lua_newuserdata(L, sizeof(*buffer));
    luaL_setmetatable(L, "Framebuffer");
    glGenFramebuffers(1, buffer);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Framebuffer, gc)
    glDeleteFramebuffers(1, self);
END_FUNCTION(1)

static BEGIN_FUNCTION(gl, BindFramebuffer)
    int target = (int)luaL_checkinteger(L, 1);
    Framebuffer* buffer = luaL_testudata(L, 2, "Framebuffer");
    if (buffer) glBindFramebuffer(target, *buffer);
    else glBindFramebuffer(target, 0);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, AttachTexture2D)
    int target = (int)luaL_checkinteger(L, 1);
    int attachment = (int)luaL_checkinteger(L, 2);
    int tex_target = (int)luaL_checkinteger(L, 3);
    Texture* tex = luaL_checkudata(L, 4, "Texture");
    glFramebufferTexture2D(target, attachment, tex_target, *tex, 0);
END_FUNCTION(0)

static BEGIN_META(Framebuffer)
    BEGIN_REG(Framebuffer)
        REG_META_FIELD(Framebuffer, gc),
    END_REG()
    NEW_META(Framebuffer);
END_META(1)

/************************
 #                      #
 #     Vertex Array     #
 #                      #
 ************************/

static BEGIN_FUNCTION(gl, NewVertexArray)
    VertexArray* vao = lua_newuserdata(L, sizeof(*vao));
    luaL_setmetatable(L, "VertexArray");
#if !defined(__EMSCRIPTEN__)
    glGenVertexArrays(1, vao);
#endif
END_FUNCTION(1)

static BEGIN_META_FUNCTION(VertexArray, gc)
#if !defined(__EMSCRIPTEN__)
    glDeleteVertexArrays(1, self);
#endif
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, BindVertexArray)
    VertexArray* vao = luaL_testudata(L, 1, "VertexArray");
#if !defined(__EMSCRIPTEN__)
    if (vao) glBindVertexArray(*vao);
    else glBindVertexArray(0);
#endif
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, EnableVertexAttribArray)
    int attrib = (int)luaL_checkinteger(L, 1);
    glEnableVertexAttribArray(attrib);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, DisableVertexAttribArray)
    int attrib = (int)luaL_checkinteger(L, 1);
    glDisableVertexAttribArray(attrib);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, VertexAttribPointer)
    int attrib = (int)luaL_checkinteger(L, 1);
    int size = (int)luaL_checkinteger(L, 2);
    int type = (int)luaL_checkinteger(L, 3);
    int normalized = lua_toboolean(L, 4);
    int stride = (int)luaL_checkinteger(L, 5);
    int offset = (int)luaL_checkinteger(L, 6);
    glVertexAttribPointer(attrib, size, type, normalized, stride, (void*)offset);
END_FUNCTION(0)

static BEGIN_META(VertexArray)
    BEGIN_REG(VertexArray)
    REG_META_FIELD(VertexArray, gc),
    END_REG()
    NEW_META(VertexArray);
END_META(1)

/************************
 #                      #
 #        Buffer        #
 #                      #
 ************************/
static BEGIN_FUNCTION(gl, NewBuffer)
    NEW_UDATA(Buffer, buffer);
    glGenBuffers(1, buffer);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Buffer, gc)
    glDeleteBuffers(1, self);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, BindBuffer)
    CHECK_INTEGER(target);
    TEST_UDATA(Buffer, b);
    if (b) glBindBuffer(target, *b);
    else glBindBuffer(target, 0);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, BufferData)
    int target = (int)luaL_checkinteger(L, 1);
    int size = (int)luaL_checkinteger(L, 2);
    int usage = (int)luaL_checkinteger(L, 3);
    Data* data = luaL_testudata(L, 4, STR(Data));
    if (data) glBufferData(target, size, data->data, usage);
    else glBufferData(target, size, NULL, usage);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, BufferSubData)
    CHECK_INTEGER(target);
    CHECK_INTEGER(start);
    CHECK_INTEGER(size);
    CHECK_UDATA(Data, data);
    glBufferSubData(target, start, size, data->data);
END_FUNCTION(0)

static BEGIN_META(Buffer)
    BEGIN_REG(Buffer)
    REG_META_FIELD(Buffer, gc),
    END_REG()
    NEW_META(Buffer);
END_META(1)

/************************
 #                      #
 #        Shader        #
 #                      #
 ************************/
static BEGIN_FUNCTION(gl, NewShader)
    int gl_enum = (int)luaL_checkinteger(L, 1);
    NEW_UDATA(Shader, s);
    *s = glCreateShader(gl_enum);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Shader, gc)
    glDeleteShader(*self);
    *self = 0;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Shader, Source)
    CHECK_STRING(source);
    glShaderSource(*self, 1, &source, NULL);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Shader, Compile)
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
END_FUNCTION(0)

static BEGIN_META(Shader)
    BEGIN_REG(Shader)
        REG_META_FIELD(Shader, Source),
        REG_META_FIELD(Shader, Compile),
        REG_META_FIELD(Shader, gc),
    END_REG()
    NEW_META(Shader);
END_META(1)

/************************
 #                      #
 #       Program        #
 #                      #
 ************************/
static BEGIN_FUNCTION(gl, NewProgram)
    Program* p = lua_newuserdata(L, sizeof(*p));
    luaL_setmetatable(L, "Program");
    *p = glCreateProgram();
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Program, AttachShader)
    int args = lua_gettop(L);
    while (arg < args) {
        CHECK_UDATA(Shader, s);
        glAttachShader(*self, *s);
    }
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Program, Link)
    glLinkProgram(*self);
    int success = 0;
    glGetProgramiv(*self, GL_LINK_STATUS, &success);
    if (!success) {
        int len;
        glGetProgramiv(*self, GL_INFO_LOG_LENGTH, &len);
    #if defined(OS_WIN)
        if (len+1 > s_aux_data->size) {
            s_aux_data->size = (len+1)*2;
            s_aux_data->data = realloc(s_aux_data->data, s_aux_data->size);
        }
        char* log = s_aux_data->data;
    #else
        char log[len+1];
    #endif
        glGetProgramInfoLog(*self, len, NULL, log);
        log[len] = '\0';
        return luaL_error(L, "Failed to link program: %s\n", log);
    }
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, UseProgram)
    TEST_UDATA(Program, p);
    if (p) glUseProgram(*p);
    else glUseProgram(0);
END_FUNCTION(0)

static BEGIN_META_FUNCTION(Program, GetAttribLocation)
    const char* name = luaL_checkstring(L, 2);
    int loc = glGetAttribLocation(*self, name);
    lua_pushinteger(L, loc);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Program, GetUniformLocation)
    const char* name = luaL_checkstring(L, 2);
    int loc = glGetUniformLocation(*self, name);
    lua_pushinteger(L, loc);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(Program, gc)
    glDeleteProgram(*self);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, Uniform1fv)
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
        values[i] = (float)luaL_checknumber(L, 2+i);
    }
    glUniform1fv(location, args, values);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, Uniform2fv)
    int location = (int)luaL_checkinteger(L, 1);
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
            v[j++] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 2;
    }
    glUniform2fv(location, args, values);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, Uniform3fv)
    int location = (int)luaL_checkinteger(L, 1);
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
            v[j++] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 3;
    }
    glUniform3fv(location, args, values);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, Uniform4fv)
    int location = (int)luaL_checkinteger(L, 1);
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
            v[j++] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        v += 4;
    }
    glUniform4fv(location, args, values);
END_FUNCTION(0)

static BEGIN_FUNCTION(gl, UniformMatrix4fv)
    CHECK_INTEGER(location);
    CHECK_INTEGER(count);
    GET_BOOLEAN(normalize);
    Mat4* m = luaL_checkudata(L, arg++, "Mat4");
    glUniformMatrix4fv(location, count, normalize, **m);
END_FUNCTION(0)


static BEGIN_META(Program)
    BEGIN_REG(Program)
    REG_META_FIELD(Program, AttachShader),
    REG_META_FIELD(Program, Link),
    REG_META_FIELD(Program, GetAttribLocation),
    REG_META_FIELD(Program, GetUniformLocation),
    REG_META_FIELD(Program, gc),
    END_REG()
    NEW_META(Program);
END_META(1)

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
#if !defined(__EMSCRIPTEN__)
    ENUM_FIELD(CLAMP_TO_BORDER, GL_),
    ENUM_FIELD(CLAMP_TO_EDGE, GL_),
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

BEGIN_MODULE(gl)
#if defined(OS_WIN)
    s_aux_data = lua_newuserdata(L, sizeof(Data));
    luaL_setmetatable(L, "Data");
    s_aux_data->offset = 0;
    s_aux_data->size = 40 * sizeof(float);
    s_aux_data->data = malloc(s_aux_data->size);
    lua_rawsetp(L, LUA_REGISTRYINDEX, s_aux_data);
#endif
    BEGIN_REG(gl)
        REG_FIELD(gl, LoadGlad),
        REG_FIELD(gl, PrintVersion),
        // Core
        REG_FIELD(gl, Viewport),
        REG_FIELD(gl, Clear),
        REG_FIELD(gl, ClearColor),
        REG_FIELD(gl, ClearDepth),
        REG_FIELD(gl, ClearStencil),
        REG_FIELD(gl, Enable),
        REG_FIELD(gl, Disable),
        REG_FIELD(gl, Scissor),
        REG_FIELD(gl, BlendFunc),
        REG_FIELD(gl, DrawArrays),
        REG_FIELD(gl, DrawElements),
        // Texture
        REG_FIELD(gl, NewTexture),
        REG_FIELD(gl, BindTexture),
        REG_FIELD(gl, TexImage2D),
        REG_FIELD(gl, TexSubImage2D),
        REG_FIELD(gl, TexParameteri),
        // Framebuffer
        REG_FIELD(gl, NewFramebuffer),
        REG_FIELD(gl, BindFramebuffer),
        REG_FIELD(gl, AttachTexture2D),
        // Vertex
        REG_FIELD(gl, NewVertexArray),
        REG_FIELD(gl, BindVertexArray),
        REG_FIELD(gl, EnableVertexAttribArray),
        REG_FIELD(gl, DisableVertexAttribArray),
        REG_FIELD(gl, VertexAttribPointer),
        // Buffer
        REG_FIELD(gl, NewBuffer),
        REG_FIELD(gl, BindBuffer),
        REG_FIELD(gl, BufferData),
        REG_FIELD(gl, BufferSubData),
        // Shader
        REG_FIELD(gl, NewShader),
        REG_FIELD(gl, UseProgram),
        // Program
        REG_FIELD(gl, NewProgram),
        REG_FIELD(gl, Uniform1fv),
        REG_FIELD(gl, Uniform2fv),
        REG_FIELD(gl, Uniform3fv),
        REG_FIELD(gl, Uniform4fv),
        REG_FIELD(gl, UniformMatrix4fv),
    END_REG()
    NEW_MODULE(gl);
    LOAD_ENUMS(gl);
    LOAD_META(Texture);
    LOAD_META(Framebuffer);
    LOAD_META(Shader);
    LOAD_META(Program);
    LOAD_META(VertexArray);
    LOAD_META(Buffer);
END_MODULE(1)
