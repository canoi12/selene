#include "../renderer.h"

extern int l_Effect2D_create(lua_State* L);

typedef struct RenderBatch2D RenderBatch2D;
struct RenderBatch2D {
    Renderer* renderer;
    SDL_Window* window;
    Uint32 vao;
    Uint32 vbo;
    int white_texture_ref;
    int default_effect_ref;
    int current_effect_ref;
    int current_texture_ref;
    struct {
        int offset, count;
        Vertex2D* data;
    } buffer;
    int last_offset;
    Vertex2D aux_vertex;
};

static void s_push_draw_command(RenderBatch2D* r, lua_State* L) {
    if (r->buffer.offset == r->last_offset) return;
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_DRAW_VERTEX;
    rc.draw.mode = GL_TRIANGLES;
    rc.draw.start = r->last_offset;
    rc.draw.count = r->buffer.offset - r->last_offset;
    rc.draw.vao = r->vao;
    // fprintf(stdout, "draw command: %d %d\n", rc.draw.start, rc.draw.count);
    r->renderer->push(r->renderer, &rc);
    r->last_offset = r->buffer.offset;
}

static int l_RenderBatch2D_create(lua_State* L) {
    INIT_ARG();
    CHECK_UDATA(Renderer, r);
    NEW_UDATA(RenderBatch2D, self);
    self->renderer = r;

    lua_pushcfunction(L, l_Effect2D_create);
    lua_pushnil(L);
    lua_pushnil(L);
    if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        return luaL_error(L, "failed to load default effect: %s", lua_tostring(L, -1));
    }
    Effect2D* effect = (Effect2D*) lua_touserdata(L, -1);
    self->default_effect_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    self->buffer.offset = 0;
    self->buffer.count = 1024;
    self->buffer.data = malloc(sizeof(Vertex2D) * self->buffer.count);
#if defined(DEBUG)
    fprintf(stdout, "Batch buffer size: %d\n", self->buffer.count * sizeof(Vertex2D));
    fprintf(stdout, "RenderCommandPool size: %d\n", sizeof(struct RenderCommandPool));
#endif

    self->last_offset = 0;

    glEnable(GL_DEPTH_TEST);

    /* Initialize VAO and VBO */
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    glGenVertexArrays(1, &self->vao);
    glBindVertexArray(self->vao);
#endif
    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * self->buffer.count, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(effect->position_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
    glVertexAttribPointer(effect->color_location, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(effect->texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    glBindVertexArray(0);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Load white texture */
    const unsigned char white_pixels[4] = { 0xff, 0xff, 0xff, 0xff };
    Uint32 tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    NEW_UDATA(Texture2D, texture);
    texture->handle = tex;
    texture->width = 1;
    texture->height = 1;
    self->white_texture_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_window_ref);
    SDL_Window** w = (SDL_Window**)lua_touserdata(L, -1);
    lua_pop(L, 1);
    self->window = *w;

    lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_effect_ref);
    self->current_effect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->white_texture_ref);
    self->current_texture_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    self->aux_vertex.z = 0.f;
    self->aux_vertex.r = 1.f;
    self->aux_vertex.g = 1.f;
    self->aux_vertex.b = 1.f;
    self->aux_vertex.a = 1.f;

    return 1;
}

static int l_RenderBatch2D__destroy(lua_State* L) {
    CHECK_META(RenderBatch2D);
    glDeleteBuffers(1, &self->vbo);

#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    glDeleteVertexArrays(1, &self->vao);
#endif

    if (self->white_texture_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self->white_texture_ref);
        Texture2D* texture = (Texture2D*)lua_touserdata(L, -1);
        glDeleteTextures(1, &texture->handle);
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, self->white_texture_ref);
    }

    if (self->default_effect_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_effect_ref);
        Effect2D* effect = (Effect2D*)lua_touserdata(L, -1);
        glDeleteProgram(effect->handle);
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, self->default_effect_ref);
    }

    if (self->buffer.data) free(self->buffer.data);

    return 0;
}

static int l_RenderBatch2D__clear(lua_State* L) {
    CHECK_META(RenderBatch2D);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_CLEAR;
    rc.clear.mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    rc.clear.color[0] = 0.f;
    rc.clear.color[1] = 0.f;
    rc.clear.color[2] = 0.f;
    rc.clear.color[3] = 1.f;
    for (int i = 2; i <= lua_gettop(L); i++) {
        rc.clear.color[i-2] = (float)luaL_checknumber(L, i);
    }
    self->renderer->push(self->renderer, &rc);
    return 0;
}

int l_RenderBatch2D__push_vertex(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (self->buffer.offset + 1 > self->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D v;
    memcpy(&(v), &(self->aux_vertex), sizeof(Vertex2D));
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
    memcpy(&(self->buffer.data[self->buffer.offset]), &v, sizeof(Vertex2D));
    self->buffer.offset += 1;
    memcpy(&(self->aux_vertex), &v, sizeof(Vertex2D));
    return 0;
}

int l_RenderBatch2D__push_point(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (self->buffer.offset + 1 > self->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    memcpy(v, &(self->aux_vertex), sizeof(Vertex2D));
    v->x = (float)luaL_checknumber(L, 2);
    v->y = (float)luaL_checknumber(L, 3);
    self->buffer.offset += 1;
    return 0;
}

int l_RenderBatch2D__push_line(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (self->buffer.offset + 2 > self->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 2; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    v[0].x = (float)luaL_checknumber(L, 2);
    v[0].y = (float)luaL_checknumber(L, 3);
    v[1].x = (float)luaL_checknumber(L, 4);
    v[1].y = (float)luaL_checknumber(L, 5);
    // memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 3);
    self->buffer.offset += 2;
    return 0;
}

int l_RenderBatch2D__push_triangle(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (self->buffer.offset + 3 > self->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 3; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    v[0].x = (float)luaL_checknumber(L, 2);
    v[0].y = (float)luaL_checknumber(L, 3);
    v[1].x = (float)luaL_checknumber(L, 4);
    v[1].y = (float)luaL_checknumber(L, 5);
    v[2].x = (float)luaL_checknumber(L, 6);
    v[2].y = (float)luaL_checknumber(L, 7);
    // memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 3);
    self->buffer.offset += 3;
    return 0;
}

int l_RenderBatch2D__push_rect(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (self->buffer.offset + 6 > self->buffer.count) {
        return luaL_error(L, "buffer overflow");
    }
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 6; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    float x, y, w, h;
    x = (float)luaL_checknumber(L, 2);
    y = (float)luaL_checknumber(L, 3);
    w = (float)luaL_checknumber(L, 4);
    h = (float)luaL_checknumber(L, 5);
    v[0].x = x;
    v[0].y = y;
    v[0].u = 0.f;
    v[0].v = 0.f;

    v[1].x = x + w;
    v[1].y = y;
    v[1].u = 1.f;
    v[1].v = 0.f;

    v[2].x = x + w;
    v[2].y = y + h;
    v[2].u = 1.f;
    v[2].v = 1.f;

    v[3].x = x;
    v[3].y = y;
    v[3].u = 0.f;
    v[3].v = 0.f;

    v[4].x = x;
    v[4].y = y + h;
    v[4].u = 0.f;
    v[4].v = 1.f;

    v[5].x = x + w;
    v[5].y = y + h;
    v[5].u = 1.f;
    v[5].v = 1.f;

    self->buffer.offset += 6;
    return 0;
}

static const Vertex2D s_cube_vertices[24] = {
    {-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f },
    { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f },
    {-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
    {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },
    { 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },

    {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    {-0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    {-0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    {-0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },

    { 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.5f,  0.5f,  -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },

    { -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
    { -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },
    { 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },
    { 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
    { 0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
    { -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
};
static const int s_cube_indices[] = {
    0, 1, 2,   0, 2, 3,      //-- front face
    4, 5, 6,   4, 6, 7,      //-- back face
    8, 9, 10,  8, 10, 11,    //-- left face
    12, 13, 14, 12, 14, 15,   //-- right face
    16, 17, 18, 16, 18, 19,   //-- top face
    20, 21, 22, 20, 22, 23,   //-- bottom face
};
static int l_RenderBatch2D__push_cube(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if ((self->buffer.offset + 36) > self->buffer.count) {
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
    arg++;
    int is_white = 0;
    if (lua_isboolean(L, arg)) {is_white = 1;}
    Vertex2D* bv = self->buffer.data + self->buffer.offset;
    static const vec2 tex_uv[] = {
        {0.f, 0.f},
        {1.f, 0.f},
        {1.f, 1.f},
        {0.f, 1.f}
    };
    for (int i = 0; i < 36; i++) {
        const int index = s_cube_indices[i];
        const Vertex2D* v = s_cube_vertices + index;
        vec4 out;
        // vec3 in = {v->x, v->y, v->z};
        glm_mat4_mulv3(m, (float*)v, 1.f, out);
        memcpy(bv+i, v, sizeof(Vertex2D));
        bv[i].x = out[0];
        bv[i].y = out[1];
        bv[i].z = out[2];
        if (is_white)
            bv[i].r = bv[i].g = bv[i].b = bv[i].a = 1.f;
        //bv[i].u = tex_uv[index%4][0];
        //bv[i].v = tex_uv[index%4][1];
    }
    self->buffer.offset += 36;
    return 0;
}

static int l_RenderBatch2D__get_white_texture(lua_State* L) {
    CHECK_META(RenderBatch2D);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->white_texture_ref);
    return 1;
}

static int l_RenderBatch2D__get_default_effect(lua_State* L) {
    CHECK_META(RenderBatch2D);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_effect_ref);
    return 1;
}

static int l_RenderBatch2D__set_color(lua_State* L) {
    CHECK_META(RenderBatch2D);
    float c[4] = {1.f, 1.f, 1.f, 1.f};
    for (int i = 0; i < lua_gettop(L)-1; i++) {
        c[i] = (float)luaL_checknumber(L, 2+i);
    }
    self->aux_vertex.r = c[0];
    self->aux_vertex.g = c[1];
    self->aux_vertex.b = c[2];
    self->aux_vertex.a = c[3];
    return 0;
}

static int l_RenderBatch2D__set_texture(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (!lua_isuserdata(L, arg) && !lua_isnil(L, arg))
        return luaL_argerror(L, arg, "must be a Texture2D, Canvas or nil");
    Texture2D* tex = (Texture2D*)lua_touserdata(L, arg);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_effect_ref);
    Texture2D* curr = (Texture2D*)lua_touserdata(L, -1);
    if (tex != curr) {
        s_push_draw_command(self, L);
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_TEXTURE;
        rc.texture.slot = 0;
        rc.texture.target = GL_TEXTURE_2D;
        rc.texture.handle = tex ? tex->handle : 0;
        self->renderer->push(self->renderer, &rc);
    }
    return 0;
}

static int l_RenderBatch2D__set_effect(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (!lua_isuserdata(L, arg) && !lua_isnil(L, arg))
        return luaL_argerror(L, arg, "must be an Effect2D or nil");
    Effect2D* eff = (Effect2D*)lua_touserdata(L, arg);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
    Effect2D* curr = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (eff != curr) {
        s_push_draw_command(self, L);
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_DRAW_VERTEX;
        rc.draw.mode = GL_TRIANGLES;
        rc.draw.start = 
        rc.type = RENDER_COMMAND_SET_EFFECT;
        rc.effect.handle = eff ? eff->handle : 0;
        self->renderer->push(self->renderer, &rc);
    }
    return 0;
}

static int l_RenderBatch2D__set_canvas(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (!lua_isuserdata(L, arg) && !lua_isnil(L, arg))
        return luaL_argerror(L, arg, "must be an Canvas or nil");
    Canvas def;
    Canvas* canvas = (Canvas*)lua_touserdata(L, arg);
    if (!canvas) {
        int w, h;
        SDL_GetWindowSize(self->window, &w ,&h);
        def.texture.width = w;
        def.texture.height = h;
        def.fbo = 0;
        def.texture.handle = 0;
        canvas = &def;
    }
    s_push_draw_command(self, L);
    struct RenderCommand rc;
    /* Set target */
    rc.type = RENDER_COMMAND_SET_TARGET;
    rc.target.target = GL_FRAMEBUFFER;
    rc.target.handle = canvas ? canvas->fbo : 0;
    self->renderer->push(self->renderer, &rc);
    /* Set viewport */
    memset(&rc, 0, sizeof(rc));
    rc.type = RENDER_COMMAND_SET_VIEWPORT;
    rc.viewport.width = canvas->texture.width;
    rc.viewport.height = canvas->texture.height;
    self->renderer->push(self->renderer, &rc);
    /* Set projection */
    memset(&rc, 0, sizeof(rc));
    rc.type = RENDER_COMMAND_SET_PROJECTION;
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_effect_ref);
    Effect2D* eff = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    rc.uniform.program = eff->handle;
    rc.uniform.location = eff->projection_location;
    mat4 m = GLM_MAT4_IDENTITY_INIT;
    glm_ortho(0, canvas->texture.width, canvas->texture.height, 0, 0, 1000, m);
    glm_mat4_udup(m, rc.uniform.m);
    self->renderer->push(self->renderer, &rc);
    return 0;
}

static int l_RenderBatch2D__begin(lua_State* L) {
    CHECK_META(RenderBatch2D);
    self->renderer->clear(self->renderer);
    self->buffer.offset = 0;
    self->last_offset = 0;
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_effect_ref);
    Effect2D* eff = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_SET_EFFECT;
    rc.effect.handle = eff->handle;
    self->renderer->push(self->renderer, &rc);
    /* Set texture */
    memset(&rc, 0, sizeof(rc));
    rc.type = RENDER_COMMAND_SET_TEXTURE;
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->white_texture_ref);
    Texture2D* tex = (Texture2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    rc.texture.handle = tex->handle;
    rc.texture.target = GL_TEXTURE_2D;
    rc.texture.slot = 0;
    self->renderer->push(self->renderer, &rc);
    return 0;
}

static void s_draw_batch(RenderBatch2D* r, lua_State* L) {
    if (!r) return;
    if (r->buffer.offset != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, r->buffer.offset*sizeof(Vertex2D), r->buffer.data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        s_push_draw_command(r, L);
    }
    r->renderer->call(r->renderer);
}

static int l_RenderBatch2D__finish(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_draw_batch(self, L);
    return 0;
}

int l_RenderBatch2D_meta(lua_State* L) {
    luaL_newmetatable(L, RENDER_BATCH2D_CLASS);
    const luaL_Reg reg[] = {
        REG_FIELD(RenderBatch2D, create),
        REG_META_FIELD(RenderBatch2D, destroy),
        REG_META_FIELD(RenderBatch2D, clear),
        REG_META_FIELD(RenderBatch2D, push_vertex),
        REG_META_FIELD(RenderBatch2D, push_triangle),
        REG_META_FIELD(RenderBatch2D, push_rect),
        REG_META_FIELD(RenderBatch2D, push_cube),
        REG_META_FIELD(RenderBatch2D, get_white_texture),
        REG_META_FIELD(RenderBatch2D, get_default_effect),
        REG_META_FIELD(RenderBatch2D, set_color),
        REG_META_FIELD(RenderBatch2D, set_texture),
        REG_META_FIELD(RenderBatch2D, set_effect),
        REG_META_FIELD(RenderBatch2D, set_canvas),
        REG_META_FIELD(RenderBatch2D, begin),
        REG_META_FIELD(RenderBatch2D, finish),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
    lua_pushcfunction(L, l_RenderBatch2D_create);
    return 1;
}
