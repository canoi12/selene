#include "selene_renderer.h"

extern int l_Effect2D_create(lua_State* L);
extern int l_Font_8x8(lua_State* L);
extern void char_rect(FontGlyph* glyphs, const int c, float *x, float *y, int* out_pos, int* out_rect, int width, int line_height);
extern int utf8_codepoint(uint8_t* p, int* codepoint);

typedef struct RenderBatch2D RenderBatch2D;
struct RenderBatch2D {
    Renderer* renderer;
    SDL_Window* window;
    RenderList* list;

    Uint32 vao;
    Uint32 vbo;
    int white_texture_ref;
    int default_effect_ref;
    int default_font_ref;

    int current_clear_mask;
    int current_draw_mode;

    int current_effect_ref;
    int current_texture_ref;
    int current_canvas_ref;

    Vertex2D aux_vertex;
    struct {
        int offset, count;
        Vertex2D* data;
    } buffer;
    int last_offset;
    
    int matrix_mode;
    mat4* matrix;
};

static void s_check_buffer(lua_State* L, RenderBatch2D* rb, int count) {
    if ((rb->buffer.offset + count) > rb->buffer.count) {
        // return luaL_error(L, "buffer overflow");
        rb->buffer.count *= 2;
        rb->buffer.data = realloc(rb->buffer.data, sizeof(Vertex2D) * rb->buffer.count);
        if (!rb->buffer.data) {
            fprintf(stderr, "Failed to realloc memory for vertex buffer\n");
            exit(EXIT_FAILURE);
        }
        glBindBuffer(GL_ARRAY_BUFFER, rb->vbo);
        glBufferData(GL_ARRAY_BUFFER, rb->buffer.count*sizeof(Vertex2D), rb->buffer.data, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

}

static void s_push_draw_command(RenderBatch2D* r, lua_State* L) {
    if (r->buffer.offset == r->last_offset) return;
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_DRAW_VERTEX;
    rc.draw.mode = draw_modes_values[r->current_draw_mode];
    rc.draw.start = r->last_offset;
    rc.draw.count = r->buffer.offset - r->last_offset;
    // fprintf(stdout, "draw command: %d %d\n", rc.draw.start, rc.draw.count);
    RENDERLIST_PUSH(r->renderer->list_ptr, &rc);
    r->last_offset = r->buffer.offset;
}

static void s_push_update_size(RenderBatch2D* r, lua_State* L, int w, int h) {
    struct RenderCommand rc;
    /* Set viewport */
    memset(&rc, 0, sizeof(rc));
    rc.type = RENDER_COMMAND_SET_VIEWPORT;
    rc.viewport.width = w;
    rc.viewport.height = h;
    RENDERLIST_PUSH(r->list, &rc);
    /* Set projection */
    memset(&rc, 0, sizeof(rc));
    rc.type = RENDER_COMMAND_SET_PROJECTION;
    lua_rawgeti(L, LUA_REGISTRYINDEX, r->current_effect_ref);
    Effect2D* eff = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    rc.uniform.program = eff->handle;
    rc.uniform.location = eff->projection_location;
    mat4 m = GLM_MAT4_IDENTITY_INIT;
    glm_ortho(0, (float)w, (float)h, 0, 0, 1000, m);
    glm_mat4_udup(m, rc.uniform.m);
    RENDERLIST_PUSH(r->list, &rc);
}

static int l_RenderBatch2D_create(lua_State* L) {
    INIT_ARG();
    CHECK_UDATA(Renderer, r);
    NEW_UDATA(RenderBatch2D, self);
    self->renderer = r;
    self->list = r->list_ptr;
    self->current_draw_mode = 2;
    self->current_clear_mask = GL_COLOR_BUFFER_BIT;
    self->matrix = malloc(2 * sizeof(mat4));
    self->matrix_mode = 0;

    lua_pushcfunction(L, l_Font_8x8);
    if (lua_pcall(L, 0, 1, 0) != LUA_OK)
        return luaL_error(L, "failed to created default font: %s", lua_tostring(L, -1));
    self->default_font_ref = luaL_ref(L, LUA_REGISTRYINDEX);

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
#endif

    self->last_offset = 0;

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

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

    if (self->matrix) free(self->matrix);
    if (self->buffer.data) free(self->buffer.data);

    return 0;
}

static int l_RenderBatch2D__clear(lua_State* L) {
    CHECK_META(RenderBatch2D);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_CLEAR;
    rc.clear.mask = self->current_clear_mask;
    rc.clear.color[0] = 0.f;
    rc.clear.color[1] = 0.f;
    rc.clear.color[2] = 0.f;
    rc.clear.color[3] = 1.f;
    for (int i = 2; i <= lua_gettop(L); i++) {
        rc.clear.color[i-2] = (float)luaL_checknumber(L, i);
    }
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int l_RenderBatch2D__enable_3d(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_push_draw_command(self, L);
    self->current_clear_mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_ENABLE;
    rc.enable.attrib = GL_DEPTH_TEST;
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int l_RenderBatch2D__disable_3d(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_push_draw_command(self, L);
    self->current_clear_mask = GL_COLOR_BUFFER_BIT;
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_DISABLE;
    rc.disable.attrib = GL_DEPTH_TEST;
    RENDERLIST_PUSH(self->list, &rc);
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

int l_RenderBatch2D__draw_point(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_check_buffer(L, self, 1);
    if (self->current_draw_mode != 0) {
        s_push_draw_command(self, L);
        self->current_draw_mode = 0;
    }
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    memcpy(v, &(self->aux_vertex), sizeof(Vertex2D));
    v->x = (float)luaL_checknumber(L, 2);
    v->y = (float)luaL_checknumber(L, 3);
    self->buffer.offset += 1;
    return 0;
}

int l_RenderBatch2D__draw_line(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_check_buffer(L, self, 2);
    if (self->current_draw_mode != 1) {
        s_push_draw_command(self, L);
        self->current_draw_mode = 1;
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

static int l_RenderBatch2D__line_triangle(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    s_check_buffer(L, self, 6);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 6; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    v[0].x = (float)luaL_checknumber(L, 2);
    v[0].y = (float)luaL_checknumber(L, 3);
    v[1].x = (float)luaL_checknumber(L, 4);
    v[1].y = (float)luaL_checknumber(L, 5);

    v[2].x = v[1].x;
    v[2].y = v[1].y;
    v[3].x = (float)luaL_checknumber(L, 6);
    v[3].y = (float)luaL_checknumber(L, 7);

    v[4].x = v[0].x;
    v[4].y = v[0].y;
    v[5].x = v[3].x;
    v[5].y = v[3].y;
    // memcpy(&(batch->buffer.data[batch->buffer.offset]), v, sizeof(Vertex2D) * 3);
    self->buffer.offset += 6;
    return 0;
}

static int l_RenderBatch2D__fill_triangle(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    s_check_buffer(L, self, 3);
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

static const lua_CFunction s_triangle_functions[] = {
    NULL,
    l_RenderBatch2D__line_triangle,
    l_RenderBatch2D__fill_triangle,
};

int l_RenderBatch2D__draw_triangle(lua_State* L) {
    CHECK_META(RenderBatch2D);
    lua_CFunction func = s_triangle_functions[self->current_draw_mode];
    if (func) return func(L);
    return 0;
}

/**
 * Rectangle
 **/

static int l_RenderBatch2D__point_rect(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    int arg = 2;
    s_check_buffer(L, self, 4);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 4; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    float x, y, w, h;
    x = (float)luaL_checknumber(L, 2);
    y = (float)luaL_checknumber(L, 3);
    w = (float)luaL_checknumber(L, 4);
    h = (float)luaL_checknumber(L, 5);
    int inv = 0;
    float yy = 0.f;
    if (lua_isboolean(L, 6)) {
        inv = lua_toboolean(L, 6);
        yy = 1.f;
    }
    v[0].x = x;
    v[0].y = y;
    v[0].u = 0.f;
    v[0].v = yy;

    v[1].x = x + w;
    v[1].y = y;
    v[1].u = 1.f;
    v[1].v = yy;

    v[2].x = x + w;
    v[2].y = y + h;
    v[2].u = 1.f;
    v[2].v = 1.f-yy;

    v[3].x = x;
    v[3].y = y + h;
    v[3].u = 0.f;
    v[3].v = 1.f-yy;

    self->buffer.offset += 4;
    return 0;
}

static int l_RenderBatch2D__line_rect(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    int arg = 2;
    s_check_buffer(L, self, 8);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 8; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    float x, y, w, h;
    x = (float)luaL_checknumber(L, 2);
    y = (float)luaL_checknumber(L, 3);
    w = (float)luaL_checknumber(L, 4);
    h = (float)luaL_checknumber(L, 5);
    int inv = 0;
    float yy = 0.f;
    if (lua_isboolean(L, 6)) {
        inv = lua_toboolean(L, 6);
        yy = 1.f;
    }
    // top
    v[0].x = x;
    v[0].y = y;
    v[0].u = 0.f;
    v[0].v = yy;

    v[1].x = x + w;
    v[1].y = y;
    v[1].u = 1.f;
    v[1].v = yy;

    // right
    v[2].x = x + w;
    v[2].y = y;
    v[2].u = 1.f;
    v[2].v = yy;

    v[3].x = x + w;
    v[3].y = y + h;
    v[3].u = 1.f;
    v[3].v = 1.f-yy;

    // bottom
    v[4].x = x + w;
    v[4].y = y + h;
    v[4].u = 1.f;
    v[4].v = 1.f-yy;

    v[5].x = x;
    v[5].y = y + h;
    v[5].u = 0.f;
    v[5].v = 1.f-yy;

    // left
    v[6].x = x;
    v[6].y = y + h;
    v[6].u = 0.f;
    v[6].v = 1.f-yy;

    v[7].x = x;
    v[7].y = y;
    v[7].u = 0.f;
    v[7].v = yy;

    self->buffer.offset += 8;
    return 0;
}

static int l_RenderBatch2D__fill_rect(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    int arg = 2;
    s_check_buffer(L, self, 6);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 6; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    if (lua_isnumber(L, arg)) {
        float x, y, w, h;
        x = (float)luaL_checknumber(L, 2);
        y = (float)luaL_checknumber(L, 3);
        w = (float)luaL_checknumber(L, 4);
        h = (float)luaL_checknumber(L, 5);
        int inv = 0;
        float yy = 0.f;
        if (lua_isboolean(L, 6)) {
            inv = lua_toboolean(L, 6);
            yy = 1.f;
        }
        v[0].x = x;
        v[0].y = y;
        v[0].u = 0.f;
        v[0].v = yy;

        v[1].x = x + w;
        v[1].y = y;
        v[1].u = 1.f;
        v[1].v = yy;

        v[2].x = x + w;
        v[2].y = y + h;
        v[2].u = 1.f;
        v[2].v = 1.f-yy;

        v[3].x = x;
        v[3].y = y;
        v[3].u = 0.f;
        v[3].v = yy;

        v[4].x = x;
        v[4].y = y + h;
        v[4].u = 0.f;
        v[4].v = 1.f-yy;

        v[5].x = x + w;
        v[5].y = y + h;
        v[5].u = 1.f;
        v[5].v = 1.f-yy;
    } else if (lua_istable(L, arg)) {
        float x, y, w, h;
        vec2 uv[4] = {
            {0.f, 0.f},
            {1.f, 0.f},
            {1.f, 1.f},
            {0.f, 1.f}
        };
        lua_rawgeti(L, arg, 1);
        x = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 2);
        y = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 3);
        w = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 4);
        h = (float)luaL_checknumber(L, -1);
        lua_pop(L, 4);

        arg++;
        if (lua_istable(L, arg)) {
            SDL_FRect rect;
            lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
            Texture2D* tex = (Texture2D*)lua_touserdata(L, -1);
            lua_pop(L, 1);
            vec2 texel;
            texel[0] = 1.f / (float)tex->width;
            texel[1] = 1.f / (float)tex->height;
            // fprintf(stdout, "tex(%d) w(%d) h(%d)\n", tex->handle, tex->width, tex->height);

            lua_rawgeti(L, arg, 1);
            rect.x = texel[0] * (float)luaL_checknumber(L, -1);
            lua_rawgeti(L, arg, 2);
            rect.y = texel[1] * (float)luaL_checknumber(L, -1);
            lua_rawgeti(L, arg, 3);
            rect.w = texel[0] * (float)luaL_checknumber(L, -1);
            lua_rawgeti(L, arg, 4);
            rect.h = texel[1] * (float)luaL_checknumber(L, -1);
            lua_pop(L, 4);

            uv[0][0] = rect.x;
            uv[0][1] = rect.y;

            uv[1][0] = rect.x+rect.w;
            uv[1][1] = rect.y;

            uv[2][0] = rect.x+rect.w;
            uv[2][1] = rect.y+rect.h;

            uv[3][0] = rect.x;
            uv[3][1] = rect.y+rect.h;
        }

        v[0].x = x;
        v[0].y = y;
        v[0].u = uv[0][0];
        v[0].v = uv[0][1];

        v[1].x = x + w;
        v[1].y = y;
        v[1].u = uv[1][0];
        v[1].v = uv[1][1];

        v[2].x = x + w;
        v[2].y = y + h;
        v[2].u = uv[2][0];
        v[2].v = uv[2][1];

        v[3].x = x;
        v[3].y = y;
        v[3].u = uv[0][0];
        v[3].v = uv[0][1];

        v[4].x = x;
        v[4].y = y + h;
        v[4].u = uv[3][0];
        v[4].v = uv[3][1];

        v[5].x = x + w;
        v[5].y = y + h;
        v[5].u = uv[2][0];
        v[5].v = uv[2][1];

    } else return luaL_argerror(L, arg, "invalid argument, number or table expected");

    self->buffer.offset += 6;
    return 0;
}

static const lua_CFunction s_rect_functions[] = {
    l_RenderBatch2D__point_rect,
    l_RenderBatch2D__line_rect,
    l_RenderBatch2D__fill_rect,
};

int l_RenderBatch2D__draw_rect(lua_State* L) {
    CHECK_META(RenderBatch2D);
    lua_CFunction func = s_rect_functions[self->current_draw_mode];
    if (func) return func(L);
    return 0;
}

/**
 * Circle
 **/

int l_RenderBatch2D__line_circle(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    int arg = 2;
    float cx = (float)luaL_checknumber(L, arg++);
    float cy = (float)luaL_checknumber(L, arg++);
    float radius = (float)luaL_checknumber(L, arg++);
    int segments = (int)luaL_optinteger(L, arg++, 16);
    s_check_buffer(L, self, 2*segments);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    float inc = M_PI2 / (float)segments;
    for (int i = 0; i < segments; i++) {
        memcpy(v, &self->aux_vertex, sizeof(Vertex2D));
        float angle = (float)i * inc;
        v[0].x = cx + (radius * cosf(angle));
        v[0].y = cy + (radius * sinf(angle)); 

        memcpy(v+1, &self->aux_vertex, sizeof(Vertex2D));
        angle = (float)(i+1) * inc;
        v[1].x = cx + (radius * cosf(angle));
        v[1].y = cy + (radius * sinf(angle)); 

        v += 2;
    }
    self->buffer.offset += 2*segments;
    return 0;
}

int l_RenderBatch2D__fill_circle(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    int arg = 2;
    float cx = (float)luaL_checknumber(L, arg++);
    float cy = (float)luaL_checknumber(L, arg++);
    float radius = (float)luaL_checknumber(L, arg++);
    int segments = (int)luaL_optinteger(L, arg++, 16);

    s_check_buffer(L, self, 3*segments);
    Vertex2D* v = self->buffer.data + self->buffer.offset;

    float inc = M_PI2 / (float)segments;
    for (int i = 0; i < segments; i++) {
        memcpy(v, &self->aux_vertex, sizeof(Vertex2D));
        v[0].x = cx;
        v[0].y = cy;

        memcpy(v+1, &self->aux_vertex, sizeof(Vertex2D));
        float angle = (float)i * inc;
        v[1].x = cx + (radius * cosf(angle));
        v[1].y = cy + (radius * sinf(angle)); 

        memcpy(v+2, &self->aux_vertex, sizeof(Vertex2D));
        angle = (float)(i+1) * inc;
        v[2].x = cx + (radius * cosf(angle));
        v[2].y = cy + (radius * sinf(angle)); 

        v += 3;
    }
    self->buffer.offset += 3*segments;
    return 0;
}

static const lua_CFunction s_circle_functions[] = {
    NULL,
    l_RenderBatch2D__line_circle,
    l_RenderBatch2D__fill_circle,
};

int l_RenderBatch2D__draw_circle(lua_State* L) {
    CHECK_META(RenderBatch2D);
    lua_CFunction func = s_circle_functions[self->current_draw_mode];
    if (func) return func(L);
    return 0;
}

int l_RenderBatch2D__draw_quad(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_check_buffer(L, self, 6);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 6; i++) {
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    }
    int args = lua_gettop(L)-1;
    if (args < 4)
        return luaL_error(L, "function expects 4 tables as argument");
    lua_rawgeti(L, arg, 1);
    v[0].x = (float)luaL_checknumber(L, -1);
    v[3].x = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 2);
    v[0].y = (float)luaL_checknumber(L, -1);
    v[3].y = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 3);
    v[0].u = (float)luaL_checknumber(L, -1);
    v[3].u = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 4);
    v[0].v = (float)luaL_checknumber(L, -1);
    v[3].v = (float)luaL_checknumber(L, -1);
    lua_pop(L, 4);

    arg++;
    lua_rawgeti(L, arg, 1);
    v[1].x = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 2);
    v[1].y = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 3);
    v[1].u = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 4);
    v[1].v = (float)luaL_checknumber(L, -1);
    lua_pop(L, 4);

    arg++;
    lua_rawgeti(L, arg, 1);
    v[2].x = (float)luaL_checknumber(L, -1);
    v[5].x = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 2);
    v[2].y = (float)luaL_checknumber(L, -1);
    v[5].y = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 3);
    v[2].u = (float)luaL_checknumber(L, -1);
    v[5].u = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 4);
    v[2].v = (float)luaL_checknumber(L, -1);
    v[5].v = (float)luaL_checknumber(L, -1);
    lua_pop(L, 4);

    arg++;
    lua_rawgeti(L, arg, 1);
    v[4].x = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 2);
    v[4].y = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 3);
    v[4].u = (float)luaL_checknumber(L, -1);
    lua_rawgeti(L, arg, 4);
    v[4].v = (float)luaL_checknumber(L, -1);
    lua_pop(L, 4);

    self->buffer.offset += 6;
    return 0;
}

static int l_RenderBatch2D__draw_sprite(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (self->current_draw_mode != 2) {
        s_push_draw_command(self, L);
        self->current_draw_mode = 2;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
    Texture2D* tex = (Texture2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    SDL_FRect src;
    if (lua_istable(L, arg)) {
        lua_rawgeti(L, arg, 1);
        src.x = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 2);
        src.y = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 3);
        src.w = (float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 4);
        src.h = (float)luaL_checknumber(L, -1);
        lua_pop(L, 4);
        arg++;
    } else {
        src.x = 0.f;
        src.y = 0.f;
        src.w = (float)tex->width;
        src.h = (float)tex->height;
    }
    vec2 uv[4];
    SDL_FRect nrect;
    nrect.x = (1.f / (float)tex->width) * src.x; 
    nrect.y = (1.f / (float)tex->height) * src.y; 
    nrect.w = (1.f / (float)tex->width) * src.w;
    nrect.h = (1.f / (float)tex->height) * src.h;

    uv[0][0] = nrect.x;
    uv[0][1] = nrect.y;

    uv[1][0] = nrect.x + nrect.w;
    uv[1][1] = nrect.y;

    uv[2][0] = nrect.x + nrect.w;
    uv[2][1] = nrect.y + nrect.h;

    uv[3][0] = nrect.x;
    uv[3][1] = nrect.y + nrect.h;

    CHECK_NUMBER(float, x);
    CHECK_NUMBER(float, y);


    float angle = luaL_optnumber(L, arg++, 0.f);
    float sx = luaL_optnumber(L, arg++, 1.f);
    float sy = luaL_optnumber(L, arg++, 1.f);
    vec2 offset = {0, 0};
    if (lua_istable(L, arg)) {
        lua_rawgeti(L, arg, 1);
        offset[0] = sx*(float)luaL_checknumber(L, -1);
        lua_rawgeti(L, arg, 2);
        offset[1] = sy*(float)luaL_checknumber(L, -1);
        lua_pop(L, 2);
    }
    arg++;
    int invert[2] = {0, 0};
    if (lua_istable(L, arg)) {
        lua_rawgeti(L, arg, 1);
        invert[0] = lua_toboolean(L, -1);
        lua_rawgeti(L, arg, 2);
        invert[1] = lua_toboolean(L, -1);
        lua_pop(L, 2);
        //fprintf(stdout, "invert: %d %d\n", invert[0], invert[1]);
        arg++;
    }

    vec2 pos[4];
    pos[0][0] = 0;
    pos[0][1] = 0;

    pos[1][0] = 1.f; //src.w;//(sx*src.w);
    pos[1][1] = 0;

    pos[2][0] = 1.f; //src.w;//(sx*src.w);
    pos[2][1] = 1.f; //src.h;//(sy*src.h);

    pos[3][0] = 0;
    pos[3][1] = 1.f; //src.h;//(sy*src.h);

    #if 1
    mat4 mat = GLM_MAT4_IDENTITY_INIT;
    glm_rotate_z(mat, DEG2RAD(angle), mat);
    glm_translate(mat, (vec3){-offset[0], -offset[1], 0.f});
    glm_scale(mat, (vec3){sx*src.w, sy*src.h, 1.f});
    #else
    mat3 mat = GLM_MAT3_IDENTITY_INIT;
    glm_scale2d(mat, (vec2){src.w*sx, src.h*sy});
    //glm_translate2d(mat, (vec2){-offset[0], -offset[1]});
    glm_rotate2d(mat, DEG2RAD(angle));
    //glm_translate2d(mat, (vec2){90.f, 95.f});
    #endif

    for (int i = 0; i < 4; i++) {
        vec3 p = {pos[i][0], pos[i][1], 0.f};
        #if 1
        glm_mat4_mulv3(mat, p, 1.f, p);
        #else
        glm_mat3_mulv(mat, p, p);
        #endif
        pos[i][0] = x+p[0];
        pos[i][1] = y+p[1];
    }
    if (invert[0]) {
        const vec2 aux = {uv[0][0], uv[2][0]};
        uv[0][0] = uv[1][0];
        uv[1][0] = aux[0];
        uv[2][0] = uv[3][0];
        uv[3][0] = aux[1];
    } 
    if (invert[1]) {
        const vec2 aux = {uv[0][1], uv[1][1]};
        uv[0][1] = uv[3][1];
        uv[3][1] = aux[0];
        uv[1][1] = uv[2][1];
        uv[2][1] = aux[1];
    }
    
    s_check_buffer(L, self, 6);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    for (int i = 0; i < 6; i++)
        memcpy(&(v[i]), &(self->aux_vertex), sizeof(Vertex2D));
    v[0].x = pos[0][0];
    v[0].y = pos[0][1];
    v[0].u = uv[0][0];
    v[0].v = uv[0][1];

    v[1].x = pos[1][0];
    v[1].y = pos[1][1];
    v[1].u = uv[1][0];
    v[1].v = uv[1][1];

    v[2].x = pos[2][0];
    v[2].y = pos[2][1];
    v[2].u = uv[2][0];
    v[2].v = uv[2][1];

    v[3].x = pos[0][0];
    v[3].y = pos[0][1];
    v[3].u = uv[0][0];
    v[3].v = uv[0][1];

    v[4].x = pos[3][0];
    v[4].y = pos[3][1];
    v[4].u = uv[3][0];
    v[4].v = uv[3][1];

    v[5].x = pos[2][0];
    v[5].y = pos[2][1];
    v[5].u = uv[2][0];
    v[5].v = uv[2][1];
    self->buffer.offset += 6;

    return 0;
}

static int l_RenderBatch2D__draw_text(lua_State* L) {
    CHECK_META(RenderBatch2D);
    CHECK_UDATA(Font, font);
    CHECK_STRING(text);
    OPT_NUMBER(float, x, 0.f);
    OPT_NUMBER(float, y, 0.f);
    float ox = x;
    float oy = y;

    float x0, y0;
    x0 = 0.f;
    y0 = 0.f;
    uint8_t* p = (uint8_t*)text;
    int w = font->texture.width;
    int h = font->texture.height;
    
    int len = 0;
    while (*p != 0) {
        int codepoint;
        int n = utf8_codepoint(p, &codepoint);
        p += n;
        len++;
    }
    p = (uint8_t*)text;
    s_check_buffer(L, self, len*6);
    Vertex2D* v = self->buffer.data + self->buffer.offset;
    while (*p != 0) {
        int codepoint;
        int n = utf8_codepoint(p, &codepoint);
        p += n;
        SDL_FRect dest;
        int pos[2];
        int src[4];
        char_rect(font->glyphs, codepoint, &x0, &y0, pos, src, 0, font->texture.height);
        dest.x = x + (float)pos[0];
        dest.y = y + (float)pos[1];
        dest.w = (float)src[2];
        dest.h = (float)src[3];
        //fprintf(stderr, "%c:%d", *p, *p);
        //fprintf(stdout, "src (%c): %d %d %d %d\n", *p, src[0], src[1], src[2], src[3]);

        SDL_FRect t;
        t.x = (float)src[0] / (float)font->texture.width;
        t.y = (float)src[1] / (float)font->texture.height;
        t.w = (float)src[2] / (float)font->texture.width;
        t.h = (float)src[3] / (float)font->texture.height;

        memcpy(v, &self->aux_vertex, sizeof(Vertex2D));
        v[0].x = dest.x;
        v[0].y = dest.y;
        v[0].u = t.x;
        v[0].v = t.y;

        memcpy(v+1, &self->aux_vertex, sizeof(Vertex2D));
        v[1].x = dest.x + dest.w;
        v[1].y = dest.y;
        v[1].u = t.x + t.w;
        v[1].v = t.y;

        memcpy(v+2, &self->aux_vertex, sizeof(Vertex2D));
        v[2].x = dest.x + dest.w;
        v[2].y = dest.y + dest.h;
        v[2].u = t.x + t.w;
        v[2].v = t.y + t.h;

        memcpy(v+3, &self->aux_vertex, sizeof(Vertex2D));
        v[3].x = dest.x;
        v[3].y = dest.y;
        v[3].u = t.x;
        v[3].v = t.y;

        memcpy(v+4, &self->aux_vertex, sizeof(Vertex2D));
        v[4].x = dest.x + dest.w;
        v[4].y = dest.y + dest.h;
        v[4].u = t.x + t.w;
        v[4].v = t.y + t.h;

        memcpy(v+5, &self->aux_vertex, sizeof(Vertex2D));
        v[5].x = dest.x;
        v[5].y = dest.y + dest.h;
        v[5].u = t.x;
        v[5].v = t.y + t.h;

        v += 6;
        self->buffer.offset += 6;
        x += src[2];
        //SDL_RenderCopyF(*self, *(font->texture), &t, &dest);
    }
    // fprintf(stderr, "\n");
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
static int l_RenderBatch2D__draw_cube(lua_State* L) {
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
        if (is_white) {
            bv[i].r = bv[i].g = bv[i].b = bv[i].a = 1.f;
        }
    }
    self->buffer.offset += 36;
    return 0;
}

int l_RenderBatch2D__fill_sphere(lua_State* L) {
    RenderBatch2D* self = (RenderBatch2D*)lua_touserdata(L, 1);
    int arg = 2;
    float cx = (float)luaL_checknumber(L, arg++);
    float cy = (float)luaL_checknumber(L, arg++);
    float cz = (float)luaL_checknumber(L, arg++);
    float radius = (float)luaL_checknumber(L, arg++);
    int stacks = (int)luaL_optinteger(L, arg++, 16); // Número de pilhas (latitude)
    int slices = (int)luaL_optinteger(L, arg++, 16); // Número de fatias (longitude)

    // Verifique se há espaço suficiente no buffer
    int totalVertices = 6 * slices * stacks; // 6 vértices por face (2 triângulos)
    s_check_buffer(L, self, totalVertices);

    Vertex2D* v = self->buffer.data + self->buffer.offset;

    float stackStep = M_PI / (float)stacks; // Angle between stacks
    float sliceStep = 2.0f * M_PI / (float)slices; // Angle between slices

    for (int i = 0; i < stacks; i++) {
        float stackAngle1 = (float)i * stackStep;
        float stackAngle2 = stackAngle1 + stackStep;

        float v1 = 1.0f - (float)i / (float)stacks;
        float v2 = 1.0f - (float)(i + 1) / (float)stacks;

        for (int j = 0; j < slices; j++) {
            float sliceAngle1 = (float)j * sliceStep;
            float sliceAngle2 = sliceAngle1 + sliceStep;

            float u1 = (float)j / (float)slices;
            float u2 = (float)(j + 1) / (float)slices;

            // Vértices para o primeiro triângulo
            v[0].x = cx + radius * sinf(stackAngle1) * cosf(sliceAngle1);
            v[0].y = cy + radius * sinf(stackAngle1) * sinf(sliceAngle1);
            v[0].z = cz + radius * cosf(stackAngle1);
            v[0].r = self->aux_vertex.r;
            v[0].g = self->aux_vertex.g;
            v[0].b = self->aux_vertex.b;
            v[0].a = self->aux_vertex.a;
            v[0].u = u1;
            v[0].v = v1;

            v[1].x = cx + radius * sinf(stackAngle2) * cosf(sliceAngle1);
            v[1].y = cy + radius * sinf(stackAngle2) * sinf(sliceAngle1);
            v[1].z = cz + radius * cosf(stackAngle2);
            v[1].r = self->aux_vertex.r;
            v[1].g = self->aux_vertex.g;
            v[1].b = self->aux_vertex.b;
            v[1].a = self->aux_vertex.a;
            v[1].u = u1;
            v[1].v = v2;

            v[2].x = cx + radius * sinf(stackAngle2) * cosf(sliceAngle2);
            v[2].y = cy + radius * sinf(stackAngle2) * sinf(sliceAngle2);
            v[2].z = cz + radius * cosf(stackAngle2);
            v[2].r = self->aux_vertex.r;
            v[2].g = self->aux_vertex.g;
            v[2].b = self->aux_vertex.b;
            v[2].a = self->aux_vertex.a;
            v[2].u = u2;
            v[2].v = v2;

            // Vertices for the second triangle
            v[3].x = cx + radius * sinf(stackAngle1) * cosf(sliceAngle1);
            v[3].y = cy + radius * sinf(stackAngle1) * sinf(sliceAngle1);
            v[3].z = cz + radius * cosf(stackAngle1);
            v[3].r = self->aux_vertex.r;
            v[3].g = self->aux_vertex.g;
            v[3].b = self->aux_vertex.b;
            v[3].a = self->aux_vertex.a;
            v[3].u = u1;
            v[3].v = v1;

            v[4].x = cx + radius * sinf(stackAngle2) * cosf(sliceAngle2);
            v[4].y = cy + radius * sinf(stackAngle2) * sinf(sliceAngle2);
            v[4].z = cz + radius * cosf(stackAngle2);
            v[4].r = self->aux_vertex.r;
            v[4].g = self->aux_vertex.g;
            v[4].b = self->aux_vertex.b;
            v[4].a = self->aux_vertex.a;
            v[4].u = u2;
            v[4].v = v2;

            v[5].x = cx + radius * sinf(stackAngle1) * cosf(sliceAngle2);
            v[5].y = cy + radius * sinf(stackAngle1) * sinf(sliceAngle2);
            v[5].z = cz + radius * cosf(stackAngle1);
            v[5].r = self->aux_vertex.r;
            v[5].g = self->aux_vertex.g;
            v[5].b = self->aux_vertex.b;
            v[5].a = self->aux_vertex.a;
            v[5].u = u2;
            v[5].v = v1;

            v += 6;
        }
    }

    self->buffer.offset += totalVertices;
    return 0;
}

static int l_RenderBatch2D__draw_sphere(lua_State* L) {
    return l_RenderBatch2D__fill_sphere(L);
}

/* Get default resources */
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

static int l_RenderBatch2D__get_default_font(lua_State* L) {
    CHECK_META(RenderBatch2D);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_font_ref);
    return 1;
}

/* Camera */
static const char* matrix_modes[] = {"view", "projection", NULL};
static int l_RenderBatch2D__set_matrix_mode(lua_State* L) {
    CHECK_META(RenderBatch2D);
    int opt = luaL_checkoption(L, arg++, "view", matrix_modes);
    self->matrix_mode = opt;
    return 0;
}

static int l_RenderBatch2D__matrix_identity(lua_State* L) {
    CHECK_META(RenderBatch2D);
    float* f = (float*)self->matrix[self->matrix_mode];
    for (int i = 0; i < 16; i++) {
        if (i % 5 == 0) f[i] = 1.f;
        else f[i] = 0.f;
    }
    return 0;
}

static int l_RenderBatch2D__matrix_translate(lua_State* L) {
    CHECK_META(RenderBatch2D);
    vec3 pos;
    pos[0] = (float)luaL_checknumber(L, arg++);
    pos[1] = (float)luaL_checknumber(L, arg++);
    pos[2] = (float)luaL_optnumber(L, arg++, 0.f);
    glm_translate(self->matrix[self->matrix_mode], pos);
    return 0;
}

static int l_RenderBatch2D__matrix_rotate(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (lua_isnumber(L, arg)) glm_rotate_x(self->matrix[self->matrix_mode], DEG2RAD((float)lua_tonumber(L, arg)), self->matrix[self->matrix_mode]);
    arg++;
    if (lua_isnumber(L, arg)) glm_rotate_y(self->matrix[self->matrix_mode], DEG2RAD((float)lua_tonumber(L, arg)), self->matrix[self->matrix_mode]);
    arg++;
    if (lua_isnumber(L, arg)) glm_rotate_z(self->matrix[self->matrix_mode], DEG2RAD((float)lua_tonumber(L, arg)), self->matrix[self->matrix_mode]);
    return 0;
}

static int l_RenderBatch2D__matrix_rotate_z(lua_State* L) {
    CHECK_META(RenderBatch2D);
    float angle = (float)luaL_checknumber(L, arg++);
    glm_rotate_z(self->matrix[self->matrix_mode], DEG2RAD((float)lua_tonumber(L, arg)), self->matrix[self->matrix_mode]);
    return 0;
}

static int l_RenderBatch2D__matrix_scale(lua_State* L) {
    CHECK_META(RenderBatch2D);
    vec3 scale;
    scale[0] = (float)luaL_checknumber(L, arg++);
    scale[1] = (float)luaL_checknumber(L, arg++);
    scale[2] = (float)luaL_optnumber(L, arg++, 0.f);
    glm_scale(self->matrix[self->matrix_mode], scale);
    return 0;
}

/* Set states */
static int l_RenderBatch2D__set_draw_mode(lua_State* L) {
    CHECK_META(RenderBatch2D);
    int opt = luaL_checkoption(L, arg++, "triangles", draw_modes);
    if (opt != self->current_draw_mode) {
        s_push_draw_command(self, L);
        self->current_draw_mode = opt;
    }
    return 0;
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

static int l_RenderBatch2D__set_z_index(lua_State* L) {
    CHECK_META(RenderBatch2D);
    self->aux_vertex.z = (float)luaL_checknumber(L, arg);
    return 0;
}

static int l_RenderBatch2D__set_texture(lua_State* L) {
    CHECK_META(RenderBatch2D);
    Texture2D* tex = NULL;
    if (lua_isnoneornil(L, arg)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self->white_texture_ref);
        tex = (Texture2D*)lua_touserdata(L, -1);
        lua_pop(L, 1);
    } else if (lua_isuserdata(L, arg)) {
        tex = (Texture2D*)lua_touserdata(L, arg);
    } else
        return luaL_argerror(L, arg, "must be a Texture2D, Canvas or nil");
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
    Texture2D* curr = (Texture2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (tex != curr) {
        s_push_draw_command(self, L);
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_TEXTURE;
        rc.texture.slot = 0;
        rc.texture.target = GL_TEXTURE_2D;
        rc.texture.handle = tex ? tex->handle : 0;
        RENDERLIST_PUSH(self->list, &rc);
        lua_pushlightuserdata(L, tex);
        lua_rawseti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
    }
    return 0;
}

static int l_RenderBatch2D__set_effect(lua_State* L) {
    CHECK_META(RenderBatch2D);
    Effect2D* eff = NULL;
    if (lua_isnoneornil(L, arg)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_effect_ref);
        eff = (Effect2D*)lua_touserdata(L, -1);
        lua_pop(L, 1);
    } else if (lua_isuserdata(L, arg)) {
        eff = (Effect2D*)luaL_checkudata(L, arg, "Effect2D");
    } else
        return luaL_argerror(L, arg, "must be a Effect2D or nil");

    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_effect_ref);
    Effect2D* curr = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (eff != curr) {
        s_push_draw_command(self, L);
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_PROGRAM;
        rc.program.handle = eff ? eff->handle : 0;
        RENDERLIST_PUSH(self->list, &rc);
        lua_pushlightuserdata(L, eff);
        lua_rawseti(L, LUA_REGISTRYINDEX, self->current_effect_ref);
    }
    return 0;
}

static int l_RenderBatch2D__set_canvas(lua_State* L) {
    CHECK_META(RenderBatch2D);
    int args = lua_gettop(L)-1;
    Canvas def;
    Canvas* canvas = NULL;
    if (lua_isnoneornil(L, arg)) {
        int w, h;
        SDL_GetWindowSize(self->window, &w ,&h);
        def.texture.width = w;
        def.texture.height = h;
        def.fbo = 0;
        def.texture.handle = 0;
        canvas = &def;
    } else if (lua_isuserdata(L, arg)) {
        canvas = (Canvas*)lua_touserdata(L, arg);
    } else
        return luaL_argerror(L, arg, "must be a Canvas or nil");

    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_canvas_ref);
    Canvas* curr = (Canvas*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (canvas != curr) {
        s_push_draw_command(self, L);
        struct RenderCommand rc;
        /* Set target */
        rc.type = RENDER_COMMAND_SET_FRAMEBUFFER;
        rc.target.target = GL_FRAMEBUFFER;
        rc.target.handle = canvas ? canvas->fbo : 0;
        RENDERLIST_PUSH(self->list, &rc);
        s_push_update_size(self, L, canvas->texture.width, canvas->texture.height);
        if (canvas == &def) lua_pushnil(L);
        else lua_pushlightuserdata(L, canvas);
        lua_rawseti(L, LUA_REGISTRYINDEX, self->current_canvas_ref);
    }
    return 0;
}

static int l_RenderBatch2D__set_viewport(lua_State* L) {
    CHECK_META(RenderBatch2D);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_SET_VIEWPORT;
    rc.viewport.x = (int)luaL_checkinteger(L, arg++);
    rc.viewport.y = (int)luaL_checkinteger(L, arg++);
    rc.viewport.width = (int)luaL_checkinteger(L, arg++);
    rc.viewport.height = (int)luaL_checkinteger(L, arg++);
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int l_RenderBatch2D__send_integer(lua_State* L) {
    CHECK_META(RenderBatch2D);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_INTEGER_UNIFORM;
    rc.uniform.location = (int)luaL_checkinteger(L, arg++);
    rc.uniform.i = (int)luaL_checknumber(L, arg++);
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int l_RenderBatch2D__send_float(lua_State* L) {
    CHECK_META(RenderBatch2D);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_FLOAT_UNIFORM;
    rc.uniform.location = (int)luaL_checkinteger(L, arg++);
    rc.uniform.f = (float)luaL_checknumber(L, arg++);
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int send_matrix(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_push_draw_command(self, L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->current_effect_ref);
    Effect2D* eff = (Effect2D*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_SET_VIEW;
    if (self->matrix_mode == 0)
        rc.uniform.location = eff->model_view_location;
    else if(self->matrix_mode == 1)
        rc.uniform.location = eff->projection_location;
    glm_mat4_udup(self->matrix[self->matrix_mode], rc.uniform.m);
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int l_RenderBatch2D__send_matrix(lua_State* L) {
    CHECK_META(RenderBatch2D);
    if (lua_gettop(L) < arg)
        return send_matrix(L);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_FLOAT_UNIFORM;
    rc.uniform.location = (int)luaL_checkinteger(L, arg++);
    if (lua_istable(L, arg)) {
        float* m = (float*)rc.uniform.m;
        for (int i = 0; i < 16; i++) {
            lua_rawgeti(L, arg, i+1);
            m[i] = (float)luaL_checknumber(L, -1);
        }
        lua_pop(L, 16);
    }
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int l_RenderBatch2D__set_clip_rect(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_push_draw_command(self, L);
    struct RenderCommand rc;
    if (lua_isnoneornil(L, arg)) {
        rc.type = RENDER_COMMAND_DISABLE_CLIP_RECT;
    } else if (lua_isinteger(L, arg)) {
        int h;
        SDL_GetWindowSize(self->window, NULL, &h);
        rc.type = RENDER_COMMAND_ENABLE_CLIP_RECT;
        CHECK_INTEGER(left);
        CHECK_INTEGER(top);
        CHECK_INTEGER(right);
        CHECK_INTEGER(bottom);
        rc.clip.x = left;
        rc.clip.y = h-top-bottom;
        rc.clip.width = right;
        rc.clip.height = bottom;
    } else
        return luaL_argerror(L, arg, "must contains the rect values (x, y, w, h), or nil to disable");
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static const char* blend_modes[] = {
    "alpha", "additive", "subtractive", "multiply", NULL
};

static int l_RenderBatch2D__set_blend_mode(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_push_draw_command(self, L);
    int opt = luaL_checkoption(L, arg, "alpha", blend_modes);
    if (opt < 0)
        return luaL_argerror(L, arg, "invalid option");
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_SET_BLEND_MODE;
    switch (opt) {
        case 0:
            rc.blend.func0 = GL_SRC_ALPHA;
            rc.blend.func1 = GL_ONE_MINUS_SRC_ALPHA;
            rc.blend.equation = GL_FUNC_ADD;
            break;
        case 1:
            rc.blend.func0 = GL_SRC_ALPHA;
            rc.blend.func1 = GL_ONE;
            rc.blend.equation = GL_FUNC_ADD;
            break;
        case 2:
            rc.blend.func0 = GL_SRC_ALPHA;
            rc.blend.func1 = GL_ONE;
            rc.blend.equation = GL_FUNC_REVERSE_SUBTRACT;
            break;
        case 3:
            rc.blend.func0 = GL_DST_COLOR;
            rc.blend.func1 = GL_ZERO;
            rc.blend.equation = GL_FUNC_ADD;
            break;
    }
    RENDERLIST_PUSH(self->list, &rc);
    return 0;
}

static int l_RenderBatch2D__push_draw(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_push_draw_command(self, L);
    return 0;
}

static int l_RenderBatch2D__begin(lua_State* L) {
    CHECK_META(RenderBatch2D);
    RENDERLIST_CLEAR(self->list);
    self->buffer.offset = 0;
    self->last_offset = 0;
    /* Set effect */
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->default_effect_ref);
    Effect2D* eff = (Effect2D*)lua_touserdata(L, -1);
    lua_rawseti(L, LUA_REGISTRYINDEX, self->current_effect_ref);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_SET_PROGRAM;
    rc.program.handle = eff->handle;
    RENDERLIST_PUSH(self->list, &rc);
    /* Set vertex array */
    memset(&rc, 0, sizeof(rc));
#if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
    rc.type = RENDER_COMMAND_SET_VERTEX_ARRAY;
    rc.vao.handle = self->vao;
#else
    rc.type = RENDER_COMMAND_SET_BUFFER;
    rc.buffer.target = GL_ARRAY_BUFFER;
    rc.buffer.handle = self->vbo;
#endif
    RENDERLIST_PUSH(self->list, &rc);
    /* Set texture */
    memset(&rc, 0, sizeof(rc));
    rc.type = RENDER_COMMAND_SET_TEXTURE;
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->white_texture_ref);
    Texture2D* tex = (Texture2D*)lua_touserdata(L, -1);
    lua_rawseti(L, LUA_REGISTRYINDEX, self->current_texture_ref);
    rc.texture.handle = tex->handle;
    rc.texture.target = GL_TEXTURE_2D;
    rc.texture.slot = 0;
    RENDERLIST_PUSH(self->list, &rc);
    
    int w, h;
    SDL_GetWindowSize(self->window, &w, &h);
    s_push_update_size(self, L, w, h);
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
    RENDERLIST_CALL(r->list);
}

static int l_RenderBatch2D__finish(lua_State* L) {
    CHECK_META(RenderBatch2D);
    s_draw_batch(self, L);
    return 0;
}

static int l_RenderBatch2D__present(lua_State* L) {
    CHECK_META(RenderBatch2D);
    SDL_GL_SwapWindow(self->window);
    return 0;
}

int l_RenderBatch2D_meta(lua_State* L) {
    luaL_newmetatable(L, RENDER_BATCH2D_CLASS);
    const luaL_Reg reg[] = {
        REG_FIELD(RenderBatch2D, create),
        REG_META_FIELD(RenderBatch2D, destroy),
        REG_META_FIELD(RenderBatch2D, clear),
        REG_META_FIELD(RenderBatch2D, enable_3d),
        REG_META_FIELD(RenderBatch2D, disable_3d),
        /* push vertices */
        REG_META_FIELD(RenderBatch2D, push_vertex),
        REG_META_FIELD(RenderBatch2D, draw_point),
        REG_META_FIELD(RenderBatch2D, draw_line),
        REG_META_FIELD(RenderBatch2D, draw_triangle),
        REG_META_FIELD(RenderBatch2D, draw_rect),
        REG_META_FIELD(RenderBatch2D, draw_circle),
        REG_META_FIELD(RenderBatch2D, draw_quad),
        REG_META_FIELD(RenderBatch2D, draw_sprite),
        REG_META_FIELD(RenderBatch2D, draw_text),
        REG_META_FIELD(RenderBatch2D, draw_cube),
        REG_META_FIELD(RenderBatch2D, draw_sphere),
        /* get default resources */
        REG_META_FIELD(RenderBatch2D, get_white_texture),
        REG_META_FIELD(RenderBatch2D, get_default_effect),
        REG_META_FIELD(RenderBatch2D, get_default_font),
        /* camera */
        REG_META_FIELD(RenderBatch2D, set_matrix_mode),
        REG_META_FIELD(RenderBatch2D, matrix_identity),
        REG_META_FIELD(RenderBatch2D, matrix_translate),
        REG_META_FIELD(RenderBatch2D, matrix_rotate),
        REG_META_FIELD(RenderBatch2D, matrix_rotate_z),
        REG_META_FIELD(RenderBatch2D, matrix_scale),
        REG_META_FIELD(RenderBatch2D, send_matrix),
        /* Set states */
        REG_META_FIELD(RenderBatch2D, set_color),
        REG_META_FIELD(RenderBatch2D, set_draw_mode),
        REG_META_FIELD(RenderBatch2D, set_z_index),
        REG_META_FIELD(RenderBatch2D, set_texture),
        REG_META_FIELD(RenderBatch2D, set_effect),
        REG_META_FIELD(RenderBatch2D, set_canvas),
        REG_META_FIELD(RenderBatch2D, set_clip_rect),
        REG_META_FIELD(RenderBatch2D, set_blend_mode),
        REG_META_FIELD(RenderBatch2D, set_viewport),
        /* uniforms */
        REG_META_FIELD(RenderBatch2D, send_integer),
        REG_META_FIELD(RenderBatch2D, send_float),
        REG_META_FIELD(RenderBatch2D, send_matrix),
        /* draw */
        REG_META_FIELD(RenderBatch2D, push_draw),
        REG_META_FIELD(RenderBatch2D, begin),
        REG_META_FIELD(RenderBatch2D, finish),
        REG_META_FIELD(RenderBatch2D, present),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
    lua_pushcfunction(L, l_RenderBatch2D_create);
    return 1;
}
