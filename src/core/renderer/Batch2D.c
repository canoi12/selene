#include "selene.h"
#include "lua_helper.h"

typedef struct Texture2D Texture2D;
struct Texture2D {
    Uint32 handle;
    int width;
    int height;
};

typedef struct Vertex2D Vertex2D;
struct Vertex2D {
    float x, y, z;
    float r, g, b, a;
    float u, v;
};

typedef struct Effect2D Effect2D;
struct Effect2D {
    Uint32 handle;
    int position_location;
    int color_location;
    int texcoord_location;
    int model_view_location;
    int projection_location;
};

const char* c_vertex_header =
#if defined(USE_GLES2)
"#version 100\n"
"attribute vec3 a_position;\n"
"attribute vec4 a_color;\n"
"attribute vec2 a_texcoord;\n"
"varying vec4 v_color;\n"
"varying vec2 v_texcoord;\n"
#else
"#version 150\n"
"in vec3 a_position;\n"
"in vec4 a_color;\n"
"in vec2 a_texcoord;\n"
"out vec4 v_color;\n"
"out vec2 v_texcoord;\n"
#endif
"uniform mat4 u_model_view;\n"
"uniform mat4 u_projection;";

const char* c_vertex_position_default =
"vec4 position(vec4 pos, mat4 view, mat4 projection) {\n"
"   return pos;\n"
"}\n";

const char* c_vertex_main =
"void main() {\n"
"   gl_Position = position(vec4(a_position, 1), u_model_view, u_projection);\n"
"   v_color = a_color;\n"
"   v_texcoord = a_texcoord;\n"
"}\n";

const char* c_fragment_header =
#if defined(USE_GLES2)
"#version 100\n"
"precision mediump float;\n"
"varying vec4 v_color;\n"
"varying vec2 v_texcoord;\n"
"#define fragColor gl_FragColor\n"
#else
"#version 150\n"
"in vec4 v_color;\n"
"in vec2 v_texcoord;\n"
"out vec4 fragColor;\n"
#endif
"uniform sampler2D u_texture;\n";

const char* c_fragment_pixel_default =
"vec4 pixel(vec4 color, sampler2D tex, vec2 texcoord) {\n"
"   return color * texture2D(tex, texcoord);\n"
"}\n";

const char* c_fragment_main =
"void main() {\n"
"   fragColor = pixel(v_color, u_texture, v_texcoord);\n"
"}\n";

int l_Effect2D__call(lua_State* L) {
    const char* position = luaL_optstring(L, 1, c_vertex_position_default);
    const char* pixel = luaL_optstring(L, 2, c_fragment_pixel_default);

    Uint32 vertex_shader, fragment_shader;
    int success = 0;

    /* Vertex shader*/
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_source[] = { c_vertex_header, position, c_vertex_main };
    glShaderSource(vertex_shader, 3, vertex_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        fprintf(stderr, "Failed to compile vertex shader: %s\n", log);
        return luaL_error(L, "Failed to compile vertex shader: %s", log);
    }

    /* Fragment shader */
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_source[] = { c_fragment_header, pixel, c_fragment_main };
    glShaderSource(fragment_shader, 3, fragment_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        fprintf(stderr, "Failed to compile fragment shader: %s\n", log);
        return luaL_error(L, "Failed to compile fragment shader: %s", log);
    }
    /* Program */
    Uint32 program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        fprintf(stderr, "Failed to link shader program: %s\n", log);
        return luaL_error(L, "Failed to link shader program: %s", log);
    }
    Effect2D* effect = (Effect2D*)lua_newuserdata(L, sizeof(Effect2D));
    luaL_setmetatable(L, "Effect2D");
    effect->handle = program;
    effect->position_location = glGetAttribLocation(program, "a_position");
    effect->color_location = glGetAttribLocation(program, "a_color");
    effect->texcoord_location = glGetAttribLocation(program, "a_texcoord");
    effect->model_view_location = glGetUniformLocation(program, "u_model_view");
    effect->projection_location = glGetUniformLocation(program, "u_projection");
    glUseProgram(program);
    float m[] = {
                    1.f, 0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f
                };
    glUniformMatrix4fv(effect->model_view_location, 1, GL_FALSE, m);
    glUniformMatrix4fv(effect->projection_location, 1, GL_FALSE, m);
    glUseProgram(0);
    return 1;
}

int l_Effect2D_destroy(lua_State* L) {
    Effect2D* effect = (Effect2D*)lua_touserdata(L, 1);
    glDeleteProgram(effect->handle);
    return 0;
}

int l_Effect2D_open_meta(lua_State* L) {
    luaL_newmetatable(L, "Effect2D");
    lua_pushcfunction(L, l_Effect2D__call);
    lua_setfield(L, -2, "__call");
    lua_pushcfunction(L, l_Effect2D_destroy);
    lua_setfield(L, -2, "destroy");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 0;
}

typedef struct Batch2D Batch2D;
struct Batch2D {
    int gl_context_ref;

    int clear_flags;
    float clear_color[4];

    Uint32 vao;
    Uint32 vbo;
    struct {
        int offset, count;
        Vertex2D* data;
    } buffer;
    Vertex2D aux_vertex;

    int white_texture_ref;
    int default_effect_ref;

    int current_texture_ref;
    int current_effect_ref;
};

int l_selene_renderer_Batch2D__call(lua_State* L) {
    Batch2D* self = (Batch2D*)lua_newuserdata(L, sizeof(Batch2D));
    luaL_setmetatable(L, "Batch2D");
    if (g_selene_context.window_ref == LUA_NOREF) {
        return luaL_error(L, "SDL window must be initialized");
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.window_ref);
    SDL_Window** win = (SDL_Window**)lua_touserdata(L, -1);
    lua_pop(L, 1);
    SDL_GLContext ctx = SDL_GL_CreateContext(*win);
    if (!ctx)
        return luaL_error(L, "failed to create SDL OpenGL context: %s", SDL_GetError());
    SDL_GLContext* gl_ctx = (SDL_GLContext*)lua_newuserdata(L, sizeof(*gl_ctx));
    luaL_setmetatable(L, "sdlGLContext");
    SDL_GL_MakeCurrent(*win, ctx);
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0) {
        return luaL_error(L, "Failed to init glad");
    }
    *gl_ctx = ctx;
    self->gl_context_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    self->clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    glEnable(GL_DEPTH_TEST);

    self->buffer.offset = 0;
    self->buffer.count = 1024;
    self->buffer.data = malloc(sizeof(Vertex2D) * self->buffer.count);
    /* Initialize VAO and VBO */
    glGenVertexArrays(1, &self->vao);
    glGenBuffers(1, &self->vbo);
    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * self->buffer.count, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Load white texture */
    unsigned char white_pixels[4] = { 0xff, 0xff, 0xff, 0xff };
    Uint32 tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    Texture2D* texture = (Texture2D*)lua_newuserdata(L, sizeof(Texture2D));
    luaL_setmetatable(L, "Texture2D");
    texture->handle = tex;
    texture->width = 1;
    texture->height = 1;
    self->white_texture_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    /* Load default effect */
    lua_pushcfunction(L, l_Effect2D__call);
    lua_pushnil(L);
    lua_pushnil(L);
    if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        return luaL_error(L, "failed to load default effect: %s", lua_tostring(L, -1));
    }
    
    self->default_effect_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 1;
}

int l_selene_Batch2D_destroy(lua_State* L) {
    Batch2D* self = (Batch2D*)lua_touserdata(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->gl_context_ref);
    SDL_GLContext* gl_ctx = (SDL_GLContext*)lua_touserdata(L, -1);
#if defined(SELENE_USE_SDL3)
    SDL_GL_DestroyContext(*gl_ctx);
#else
    SDL_GL_DeleteContext(*gl_ctx);
#endif
    lua_pop(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, self->gl_context_ref);
    glDeleteBuffers(1, &self->vbo);
    glDeleteVertexArrays(1, &self->vao);

    if (self->white_texture_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
        Texture2D* texture = (Texture2D*)lua_touserdata(L, -1);
        glDeleteTextures(1, &texture->handle);
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, self->current_texture_ref);
    }

    if (self->default_effect_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_effect_ref);
        Effect2D* effect = (Effect2D*)lua_touserdata(L, -1);
        glDeleteProgram(effect->handle);
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, self->default_effect_ref);
    }

    return 0;
}

int l_selene_Batch2D_clear(lua_State* L) {
    Batch2D* self = (Batch2D*)luaL_checkudata(L, 1, "Batch2D");
    float c[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (int i = 2; i < lua_gettop(L); i++) {
        c[i-2] = luaL_checknumber(L, i);
    }
    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(self->clear_flags);
    return 0;
}

int l_selene_Batch2D_push_vertex(lua_State* L) {
    Batch2D* batch = (Batch2D*)luaL_checkudata(L, 1, "Batch2D");
    if (batch->buffer.offset + 1 > batch->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D v;
    memcpy(&(v), &(batch->aux_vertex), sizeof(Vertex2D));
    v.x = (float)luaL_checknumber(L, 2);
    v.y = (float)luaL_checknumber(L, 3);
    v.r = (float)luaL_checknumber(L, 4);
    v.g = (float)luaL_checknumber(L, 5);
    v.b = (float)luaL_checknumber(L, 6);
    v.a = (float)luaL_checknumber(L, 7);
    v.u = (float)luaL_checknumber(L, 8);
    v.v = (float)luaL_checknumber(L, 9);
    memcpy(&(batch->buffer.data[batch->buffer.offset]), &v, sizeof(Vertex2D));
    batch->buffer.offset += 1;
    memcpy(&(batch->aux_vertex), &v, sizeof(Vertex2D));
    return 0;
}

int l_selene_Batch2D_push_point(lua_State* L) {
    Batch2D* batch = (Batch2D*)luaL_checkudata(L, 1, "Batch2D");
    if (batch->buffer.offset + 1 > batch->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D v;
    memcpy(&(v), &(batch->aux_vertex), sizeof(Vertex2D));
    v.x = (float)luaL_checknumber(L, 2);
    v.y = (float)luaL_checknumber(L, 3);
    memcpy(&(batch->buffer.data[batch->buffer.offset]), &v, sizeof(Vertex2D));
    batch->buffer.offset += 1;
    return 0;
}

int l_selene_Batch2D_push_triangle(lua_State* L) {
    Batch2D* batch = (Batch2D*)luaL_checkudata(L, 1, "Batch2D");
    if (batch->buffer.offset + 3 > batch->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D v[3];
    for (int i = 0; i < 3; i++) {
        memcpy(&(v[i]), &(batch->aux_vertex), sizeof(Vertex2D));
    }
    v[0].x = (float)luaL_checknumber(L, 2);
    v[0].y = (float)luaL_checknumber(L, 3);
    v[1].x = (float)luaL_checknumber(L, 4);
    v[1].y = (float)luaL_checknumber(L, 5);
    v[2].x = (float)luaL_checknumber(L, 6);
    v[2].y = (float)luaL_checknumber(L, 7);
    memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 3);
    batch->buffer.offset += 3;
    return 0;
}

int l_selene_Batch2D_push_rect(lua_State* L) {
    Batch2D* batch = (Batch2D*)luaL_checkudata(L, 1, "Batch2D");
    if (batch->buffer.offset + 6 > batch->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D v[6];
    for (int i = 0; i < 6; i++) {
        memcpy(&(v[i]), &(batch->aux_vertex), sizeof(Vertex2D));
    }
    float x, y, w, h;
    x = (float)luaL_checknumber(L, 2);
    y = (float)luaL_checknumber(L, 3);
    w = (float)luaL_checknumber(L, 4);
    h = (float)luaL_checknumber(L, 5);
    v[0].x = x;
    v[0].y = y;
    v[1].x = x + w;
    v[1].y = y;
    v[2].x = x + w;
    v[2].y = y + h;

    v[3].x = x;
    v[3].y = y;
    v[4].x = x;
    v[4].y = y + h;
    v[5].x = x + w;
    v[5].y = y + h;

    memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 6);
    batch->buffer.offset += 6;
    return 0;
}

int l_selene_Batch2D_open_meta(lua_State* L) {
    luaL_newmetatable(L, "Batch2D");
    lua_pushcfunction(L, l_selene_renderer_Batch2D__call);
    lua_setfield(L, -2, "__call");

    lua_newtable(L);
    lua_pushcfunction(L, l_selene_Batch2D_destroy);
    lua_setfield(L, -2, "destroy");

    lua_pushcfunction(L, l_selene_Batch2D_clear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, l_selene_Batch2D_push_vertex);
    lua_setfield(L, -2, "push_vertex");
    lua_pushcfunction(L, l_selene_Batch2D_push_point);
    lua_setfield(L, -2, "push_point");
    lua_pushcfunction(L, l_selene_Batch2D_push_triangle);
    lua_setfield(L, -2, "push_triangle");
    lua_pushcfunction(L, l_selene_Batch2D_push_rect);
    lua_setfield(L, -2, "push_rect");
    lua_setfield(L, -2, "__index");
    return 1;
}