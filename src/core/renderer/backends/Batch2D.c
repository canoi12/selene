#include "../renderer.h"

extern int l_Effect2D_create(lua_State* L);

typedef struct Batch2D_Data Batch2D_Data;
struct Batch2D_Data {
    Uint32 vao;
    Uint32 vbo;
    struct {
        int offset, count;
        Vertex2D* data;
    } buffer;
    Vertex2D aux_vertex;

    int white_texture_ref;
    int default_effect_ref;
};

static inline void s_draw_and_reset(Renderer* r, lua_State* L) {
    if (!r) return;
    Batch2D_Data* data = (Batch2D_Data*)(r->internal_data);
    if (data->buffer.offset == 0) return;
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_window_ref);
    SDL_Window** win = (SDL_Window**)lua_touserdata(L, -1);
    lua_pop(L, 1);
    int w, h;
    SDL_GetWindowSize(*win, &w, &h);
    glViewport(0, 0, w, h);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, data->buffer.offset * sizeof(Vertex2D), data->buffer.data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(data->vao);

    //fprintf(stdout, "Subdata: %d %p\n", data->buffer.offset, data->buffer.data);

    lua_rawgeti(L, LUA_REGISTRYINDEX, r->l_effect_ref);
    Effect2D* effect = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (effect) glUseProgram(effect->handle);

    lua_rawgeti(L, LUA_REGISTRYINDEX, r->l_texture_ref);
    Texture2D* texture = (Texture2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (texture) glBindTexture(GL_TEXTURE_2D, texture->handle);

    glDrawArrays(r->draw_mode, 0, data->buffer.offset);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    data->buffer.offset = 0;
}

static void s_Batch2D_begin(Renderer* r, lua_State* L) {}
static void s_Batch2D_end(Renderer* r, lua_State* L) {
    s_draw_and_reset(r, L);
}
static void s_Batch2D_on_resize(Renderer* r, lua_State* L, int w, int h) {
#if 0
    float y_fov = DEG2RAD(45);
    float aspect = (float)w/(float)h;
    float n = 0.1f;
    float f = 100.f;
    glm_perspective(y_fov, aspect, n, f, m);
#endif
    glViewport(0, 0, w, h);

    mat4 m;
    glm_ortho(0, (float)w, (float)h, 0, 0, 1000, m);
    lua_rawgeti(L, LUA_REGISTRYINDEX, r->l_effect_ref);
    Effect2D* eff = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    glUseProgram(eff->handle);
    glUniformMatrix4fv(eff->projection_location, 1, GL_FALSE, m[0]);
    glUseProgram(0);

}

int l_renderer_create_Batch2D(lua_State* L) {
    SDL_Window** win = (SDL_Window**)luaL_checkudata(L, 2, "sdlWindow");
    SDL_GLContext ctx = SDL_GL_CreateContext(*win);
    if (!ctx)
        return luaL_error(L, "failed to create SDL OpenGL context: %s", SDL_GetError());
    SDL_GLContext* gl_ctx = (SDL_GLContext*)lua_newuserdata(L, sizeof(*gl_ctx));
    luaL_setmetatable(L, "sdlGLContext");
    SDL_GL_MakeCurrent(*win, ctx);
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
        return luaL_error(L, "Failed to init glad");

    *gl_ctx = ctx;
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    NEW_UDATA(Renderer, self);
    self->gl_context_ref = ref;
    self->internal_data = malloc(sizeof(Batch2D_Data));
    self->clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    self->draw_mode = GL_TRIANGLES;
    glEnable(GL_DEPTH_TEST);

    Batch2D_Data* batch_data = (Batch2D_Data*)self->internal_data;
    batch_data->buffer.offset = 0;
    batch_data->buffer.count = 1024;
    batch_data->buffer.data = malloc(sizeof(Vertex2D) * batch_data->buffer.count);
    /* Load default effect */
    lua_pushcfunction(L, l_Effect2D_create);
    lua_pushnil(L);
    lua_pushnil(L);
    if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        return luaL_error(L, "failed to load default effect: %s", lua_tostring(L, -1));
    }
    Effect2D* effect = (Effect2D*) lua_touserdata(L, -1);
    batch_data->default_effect_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    /* Initialize VAO and VBO */
    glGenVertexArrays(1, &batch_data->vao);
    glGenBuffers(1, &batch_data->vbo);
    glBindVertexArray(batch_data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * batch_data->buffer.count, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(effect->position_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
    glVertexAttribPointer(effect->color_location, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(effect->texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(7 * sizeof(float)));
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    Texture2D* texture = (Texture2D*)lua_newuserdata(L, sizeof(Texture2D));
    luaL_setmetatable(L, "Texture2D");
    texture->handle = tex;
    texture->width = 1;
    texture->height = 1;
    batch_data->white_texture_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    batch_data->aux_vertex = (Vertex2D){0, 0, 0, 1, 1, 1, 1, 0, 0};

    lua_rawgeti(L, LUA_REGISTRYINDEX, batch_data->white_texture_ref);
    self->l_texture_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(L, LUA_REGISTRYINDEX, batch_data->default_effect_ref);
    self->l_effect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    glUseProgram(effect->handle);
    #if 0
    mat4 a, b;
    glm_mat4_identity(a);
    vec3 pos = {128, 64, 13};
    glm_translate(a, pos);
    vec3 scale = {32, 64, 32};
    glm_rotate_y(a, DEG2RAD(45), b);
    glm_scale(b, scale);
    //glm_mat4_scale(b, 32);
    #else
    mat4 b = GLM_MAT4_IDENTITY_INIT;
    #endif
    glUniformMatrix4fv(effect->model_view_location, 1, GL_FALSE, b[0]);
    glUseProgram(0);
    s_Batch2D_on_resize(self, L, 640, 380);

    self->present = s_Batch2D_end;
    self->on_resize = s_Batch2D_on_resize;

    return 1;
}

int l_renderer_Batch2D_destroy(lua_State* L) {
    CHECK_META(Renderer);
    Batch2D_Data* bdata = (Batch2D_Data*)self->internal_data;
    glDeleteBuffers(1, &bdata->vbo);
    glDeleteVertexArrays(1, &bdata->vao);

    if (bdata->white_texture_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, bdata->white_texture_ref);
        Texture2D* texture = (Texture2D*)lua_touserdata(L, -1);
        glDeleteTextures(1, &texture->handle);
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, bdata->white_texture_ref);
    }

    if (bdata->default_effect_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, bdata->default_effect_ref);
        Effect2D* effect = (Effect2D*)lua_touserdata(L, -1);
        glDeleteProgram(effect->handle);
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, bdata->default_effect_ref);
    }

    if (bdata->buffer.data) {
        free(bdata->buffer.data);
    }
    free(bdata);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->gl_context_ref);
    SDL_GLContext* gl_ctx = (SDL_GLContext*)lua_touserdata(L, -1);
#if defined(SELENE_USE_SDL3)
    SDL_GL_DestroyContext(*gl_ctx);
#else
    SDL_GL_DeleteContext(*gl_ctx);
#endif
    lua_pop(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, self->gl_context_ref);

    return 0;
}

static int l_renderer_Batch2D_clear(lua_State* L) {
    CHECK_META(Renderer);
    s_draw_and_reset(self, L);
    float c[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        for (int i = 2; i <= lua_gettop(L); i++) {
        c[i-2] = (float)luaL_checknumber(L, i);
    }
    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(self->clear_flags);
    return 0;
}

static int l_renderer_Batch2D_set_draw_mode(lua_State* L) {
    CHECK_META(Renderer);
    s_draw_and_reset(self, L);
    int opt = luaL_checkoption(L, arg, "triangles", draw_modes);
    self->draw_mode = draw_modes_values[opt];
    return 0;
}

int l_renderer_Batch2D_push_vertex(lua_State* L) {
    CHECK_META(Renderer);
    Batch2D_Data* batch = (Batch2D_Data*)self->internal_data;
    if (batch->buffer.offset + 1 > batch->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D v;
    memcpy(&(v), &(batch->aux_vertex), sizeof(Vertex2D));
    v.x = (float)luaL_checknumber(L, arg++);
    v.y = (float)luaL_checknumber(L, arg++);
    v.z = (float)luaL_checknumber(L, arg++);
    v.r = (float)luaL_checknumber(L, arg++);
    v.g = (float)luaL_checknumber(L, arg++);
    v.b = (float)luaL_checknumber(L, arg++);
    v.a = (float)luaL_checknumber(L, arg++);
    v.u = (float)luaL_checknumber(L, arg++);
    v.v = (float)luaL_checknumber(L, arg++);
    //fprintf(stdout, "%f %f %f %f %f %f %f %f %f\n", v.x, v.y, v.z, v.r, v.g, v.b, v.a, v.u, v.v);
    memcpy(&(batch->buffer.data[batch->buffer.offset]), &v, sizeof(Vertex2D));
    batch->buffer.offset += 1;
    memcpy(&(batch->aux_vertex), &v, sizeof(Vertex2D));
    return 0;
}

int l_renderer_Batch2D_push_point(lua_State* L) {
    CHECK_META(Renderer);
    Batch2D_Data* batch = (Batch2D_Data*)self->internal_data;
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

int l_renderer_Batch2D_push_triangle(lua_State* L) {
    CHECK_META(Renderer);
    Batch2D_Data* batch = (Batch2D_Data*)self->internal_data;
    if (batch->buffer.offset + 3 > batch->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D* v = batch->buffer.data + batch->buffer.offset;
    for (int i = 0; i < 3; i++) {
        memcpy(&(v[i]), &(batch->aux_vertex), sizeof(Vertex2D));
    }
    v[0].x = (float)luaL_checknumber(L, 2);
    v[0].y = (float)luaL_checknumber(L, 3);
    v[1].x = (float)luaL_checknumber(L, 4);
    v[1].y = (float)luaL_checknumber(L, 5);
    v[2].x = (float)luaL_checknumber(L, 6);
    v[2].y = (float)luaL_checknumber(L, 7);
    // memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 3);
    batch->buffer.offset += 3;
    return 0;
}

int l_renderer_Batch2D_push_rect(lua_State* L) {
    CHECK_META(Renderer);
    Batch2D_Data* batch = (Batch2D_Data*)self->internal_data;
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

static const Vertex2D s_cube_vertices[24] = {
    {-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    {-0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    {-0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    {-0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },

    { 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },

    { -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
};
static const int s_cube_indices[] = {
    0, 1, 2,   0, 2, 3,      //-- front face
    4, 5, 6,   4, 6, 7,      //-- back face
    8, 9, 10,  8, 10, 11,    //-- left face
    12, 13, 14, 12, 14, 15,   //-- right face
    16, 17, 18, 16, 18, 19,   //-- top face
    20, 21, 22, 20, 22, 23,   //-- bottom face
};
static int l_renderer_Batch2D_push_cube(lua_State* L) {
    CHECK_META(Renderer);
    Batch2D_Data* batch = (Batch2D_Data*)self->internal_data;
    if (batch->buffer.offset + 36 > batch->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    mat4 m = GLM_MAT4_IDENTITY_INIT;
    if (lua_istable(L, arg)) {
        vec3 pos;
        lua_rawgeti(L, arg, 1);
        pos[0] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 2);
        pos[1] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 3);
        pos[2] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm_translate(m, pos);

    }
    arg++;
    if (lua_istable(L, arg)) {
        vec3 rotate;
        lua_rawgeti(L, arg, 1);
        rotate[0] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 2);
        rotate[1] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 3);
        rotate[2] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        mat4 out;
        glm_mat4_copy(m, out);
        glm_rotate_x(out, rotate[0], m);
        glm_rotate_y(m, rotate[1], out);
        glm_rotate_z(out, rotate[2], m);
    }
    arg++;
    if (lua_istable(L, arg)) {
        vec3 scale;
        lua_rawgeti(L, arg, 1);
        scale[0] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 2);
        scale[1] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, arg, 3);
        scale[2] = lua_tonumber(L, -1);
        lua_pop(L, 1);
        glm_scale(m, scale);
    }
    Vertex2D* bv = batch->buffer.data + batch->buffer.offset;
    for (int i = 0; i < 36; i++) {
        const Vertex2D* v = s_cube_vertices + s_cube_indices[i];
        vec3 in = {v->x, v->y, v->z};
        vec3 out;
        glm_mat4_mulv3(m, in, 1.f, out);
        memcpy(bv+i, v, sizeof(Vertex2D));
        bv[i].x = out[0];
        bv[i].y = out[1];
        bv[i].z = out[2];
    }
    batch->buffer.offset += 36;
    return 0;
}

static int l_renderer_Batch2D_set_texture(lua_State* L) {
    CHECK_META(Renderer);
    s_draw_and_reset(self, L);
    if (!lua_isuserdata(L, arg) && !lua_isnil(L, arg))
        return luaL_argerror(L, arg, "must be a Texture2D or nil");
    TEST_UDATA(Texture2D, tex);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->l_texture_ref);
    Texture2D* curr_tex = (Texture2D*)lua_touserdata(L, -1);
    if (tex != curr_tex) {
        lua_pushvalue(L, 2);
        lua_rawseti(L, LUA_REGISTRYINDEX, self->l_texture_ref);
    }
    lua_pop(L, 1);
    return 0;
}

static int l_renderer_Batch2D_set_effect(lua_State* L) {
    CHECK_META(Renderer);
    s_draw_and_reset(self, L);
    if (!lua_isuserdata(L, arg) && !lua_isnil(L, arg))
        return luaL_argerror(L, arg, "must be an Effect2D or nil");
    lua_pushvalue(L, arg);
    lua_rawseti(L, LUA_REGISTRYINDEX, self->l_effect_ref);
    return 0;
}

static int l_renderer_Batch2D_set_projection(lua_State* L) { return 0; }

int l_Batch2D_open_meta(lua_State* L) {
    luaL_newmetatable(L, "Renderer");
    const luaL_Reg reg[] = {
        REG_FIELD(renderer_Batch2D, destroy),
        REG_FIELD(renderer_Batch2D, clear),
        REG_FIELD(renderer_Batch2D, set_draw_mode),
        REG_FIELD(renderer_Batch2D, push_vertex),
        REG_FIELD(renderer_Batch2D, push_point),
        REG_FIELD(renderer_Batch2D, push_triangle),
        REG_FIELD(renderer_Batch2D, push_rect),
        REG_FIELD(renderer_Batch2D, push_cube),
        REG_FIELD(renderer_Batch2D, set_texture),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
