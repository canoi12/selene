#include "renderer.h"

const char* enable_attribs[] = { "blend", "depth test", NULL };
const int enable_attribs_values[] = { GL_BLEND, GL_DEPTH_TEST };

const char* pixel_formats[] = { "rgb", "rgba", NULL };
const int pixel_formats_values[] = { GL_RGB, GL_RGBA };

const char* texture_filters[] = { "nearest", "linear", NULL };
const int texture_filters_values[] = { GL_NEAREST, GL_LINEAR };

const char* texture_targets[] = { "tex2d", "tex3d", NULL };
const int texture_targets_values[] = { GL_TEXTURE_2D, GL_TEXTURE_3D };

const char* draw_modes[] = {"points", "lines", "triangles", NULL};
const int draw_modes_values[] = {GL_POINTS, GL_LINES, GL_TRIANGLES};

extern int l_RenderList_meta(lua_State* L);
extern int l_RenderList_create(lua_State* L);

extern int l_Effect2D_open_meta(lua_State* L);
extern int l_Texture2D_open_meta(lua_State* L);
extern int l_Texture2D_create(lua_State* L);

extern int l_Canvas_open_meta(lua_State* L);
extern int l_Canvas_create(lua_State* L);

extern int l_RenderBatch2D_meta(lua_State* L);
extern int l_RenderBatch2D_create(lua_State* L);

static void s_renderer_present(Renderer* r, lua_State* L) {
    // fprintf(stdout, "render present\n");
    lua_rawgeti(L, LUA_REGISTRYINDEX, r->l_window_ref);
    SDL_Window **window = (SDL_Window **)lua_touserdata(L, -1);
    lua_pop(L, 1);
    SDL_GL_SwapWindow(*window);
}
#if 0
int g_init_renderer(lua_State* L, Renderer* r, SDL_Window* win) {
    if (!r) {
        lua_pushstring(L, "empty renderer");
        return -1;
    }
    memset(r, 0, sizeof(*r));
    r->pool = &(r->root);
    r->pool->prev = NULL;
    r->pool->next = NULL;
    r->clear = s_renderer_clear_commands;
    r->push = s_renderer_push_command;
    r->pop = s_renderer_pop_command;
    r->call = s_renderer_call_commands;
    r->present = s_renderer_present;
    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        lua_pushfstring(L, "failed to create SDL OpenGL context: %s", SDL_GetError());
        return -1;
    }
    SDL_GLContext* gl_ctx = (SDL_GLContext*)lua_newuserdata(L, sizeof(*gl_ctx));
    luaL_setmetatable(L, "sdlGLContext");
    SDL_GL_MakeCurrent(win, ctx);
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
        return luaL_error(L, "Failed to init glad");
#endif
    r->l_gl_context_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    r->l_window_ref = g_selene_context.l_window_ref;
    return 0;
}
#endif

int l_renderer_create(lua_State* L) {
    INIT_ARG();
    SDL_Window** win = (SDL_Window**)luaL_checkudata(L, arg++, "sdlWindow");
    SDL_GLContext ctx = SDL_GL_CreateContext(*win);
    if (!ctx) {
        lua_pushfstring(L, "failed to create SDL OpenGL context: %s", SDL_GetError());
        return -1;
    }
    NEW_UDATA(Renderer, r);
    r->window_ptr = *win;
    SDL_GLContext* gl_ctx = (SDL_GLContext*)lua_newuserdata(L, sizeof(*gl_ctx));
    luaL_setmetatable(L, "sdlGLContext");
    SDL_GL_MakeCurrent(*win, ctx);
#if !defined(OS_ANDROID) && !defined(OS_EMSCRIPTEN)
    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
        return luaL_error(L, "Failed to init glad");
#endif
    lua_pushcfunction(L, l_RenderList_create);
    lua_pushvalue(L, 2);
    lua_call(L, 1, 1);
    r->list_ptr = (RenderList*)lua_touserdata(L, -1);
    fprintf(stdout, "render list: %p\n", r->list_ptr);
    r->l_render_list_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    r->l_gl_context_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, 2);
    r->l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 1;
}

static int l_Renderer__destroy(lua_State* L) {
    CHECK_META(Renderer);
    luaL_unref(L, LUA_REGISTRYINDEX, self->l_window_ref);
    if (self->l_gl_context_ref != LUA_NOREF && self->l_gl_context_ref != LUA_REFNIL) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, self->l_gl_context_ref);
        SDL_GLContext* ctx = (SDL_GLContext*)lua_touserdata(L, -1);
        lua_pop(L, 1);
        SDL_GL_DeleteContext(*ctx);
        luaL_unref(L, LUA_REGISTRYINDEX, self->l_gl_context_ref);
    }
    return 0;
}

static int l_Renderer__begin(lua_State* L) {
    CHECK_META(Renderer);
    RENDERLIST_CLEAR(self->list_ptr);
    return 0;
}

static int l_Renderer__finish(lua_State* L) {
    CHECK_META(Renderer);
    RENDERLIST_CALL(self->list_ptr);
    return 0;
}

static int l_Renderer__clear(lua_State* L) {
    CHECK_META(Renderer);
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
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__enable(lua_State* L) {
    CHECK_META(Renderer);
    int opt = luaL_checkoption(L, arg++, NULL, enable_attribs);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_ENABLE;
    rc.enable.attrib = enable_attribs_values[opt];
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__disable(lua_State* L) {
    CHECK_META(Renderer);
    int opt = luaL_checkoption(L, arg++, NULL, enable_attribs);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_DISABLE;
    rc.enable.attrib = enable_attribs_values[opt];
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__set_texture(lua_State* L) {
    CHECK_META(Renderer);
    int target = luaL_checkoption(L, arg++, "tex2d", texture_targets);
    Texture2D* tex = NULL;
    if (lua_isuserdata(L, arg)) {
        tex = (Texture2D*)lua_touserdata(L, arg);
    }
    int id = tex ? tex->handle : 0;
    if (id != self->current_tex2d_id) {
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_TEXTURE;
        rc.texture.slot = 0;
        rc.texture.target = texture_targets_values[target];
        rc.texture.handle = id;
        RENDERLIST_PUSH(self->list_ptr, &rc);
    }

    return 0;
}

static int l_Renderer__set_texture2d(lua_State* L) {
    CHECK_META(Renderer);
    Texture2D* tex = NULL;
    if (lua_isuserdata(L, arg)) {
        tex = (Texture2D*)lua_touserdata(L, arg);
    }
    int id = tex ? tex->handle : 0;
    if (id != self->current_tex2d_id) {
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_TEXTURE;
        rc.texture.slot = 0;
        rc.texture.target = GL_TEXTURE_2D;
        rc.texture.handle = id;
        RENDERLIST_PUSH(self->list_ptr, &rc);
    }
    return 0;
}

static int l_Renderer__set_framebuffer(lua_State* L) {
    CHECK_META(Renderer);
    Canvas* c = NULL;
    if (lua_isuserdata(L, arg)) {
        c = (Canvas*)lua_touserdata(L, arg);
    }
    int id = c ? c->fbo : 0;
    if (id != self->current_fbo_id) {
        struct RenderCommand rc;
        /* Set target */
        rc.type = RENDER_COMMAND_SET_TARGET;
        rc.target.target = GL_FRAMEBUFFER;
        rc.target.handle = id;
        RENDERLIST_PUSH(self->list_ptr, &rc);
    }
    return 0;
}

static int l_Renderer__set_program(lua_State* L) {
    CHECK_META(Renderer);
    Effect2D* prog = NULL;
    if (lua_isuserdata(L, arg)) {
        prog = (Effect2D*)lua_touserdata(L, arg);
    }
    int id = prog ? prog->handle : 0;
    if (id != self->current_program_id) {
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_EFFECT;
        rc.effect.handle = id;
        RENDERLIST_PUSH(self->list_ptr, &rc);
    }
    return 0;
}

static int l_Renderer__set_viewport(lua_State* L) {
    CHECK_META(Renderer);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_SET_VIEWPORT;
    rc.viewport.x = (int)luaL_checkinteger(L, arg++);
    rc.viewport.y = (int)luaL_checkinteger(L, arg++);
    rc.viewport.width = (int)luaL_checkinteger(L, arg++);
    rc.viewport.height = (int)luaL_checkinteger(L, arg++);
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__set_scissor(lua_State* L) {
    CHECK_META(Renderer);
    struct RenderCommand rc;
    if (lua_isinteger(L, arg)) {
        rc.type = RENDER_COMMAND_ENABLE_CLIP_RECT;
        CHECK_INTEGER(left);
        CHECK_INTEGER(top);
        CHECK_INTEGER(right);
        CHECK_INTEGER(bottom);
        rc.clip.x = left;
        rc.clip.y = top;
        rc.clip.width = right;
        rc.clip.height = bottom;
    } else if (lua_isnil(L, arg) || lua_gettop(L) < 2) {
        rc.type = RENDER_COMMAND_DISABLE_CLIP_RECT;
    } else
        return luaL_argerror(L, arg, "must contains the rect values (x, y, w, h), or nil to disable");
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static const char* blend_modes[] = {
    "alpha", "additive", "subtractive", "multiply", NULL
};

static int l_Renderer__set_blend_mode(lua_State* L) {
    CHECK_META(Renderer);
    int opt = luaL_checkoption(L, arg, "alpha", blend_modes);
    if (opt < 0)
        return luaL_argerror(L, arg, "invalid option");
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_SET_BLEND_MODE;
    switch(opt) {
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
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__draw(lua_State* L) {
    CHECK_META(Renderer);
    int opt = luaL_checkoption(L, arg++, "triangles", draw_modes);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_DRAW_VERTEX;
    rc.draw.mode = draw_modes_values[opt];
    rc.draw.start = luaL_checkinteger(L, arg++);
    rc.draw.count = luaL_checkinteger(L, arg++);
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer_meta(lua_State* L) {
    luaL_newmetatable(L, "Renderer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg reg[] = {
        REG_META_FIELD(Renderer, destroy),
        REG_META_FIELD(Renderer, begin),
        REG_META_FIELD(Renderer, finish),
        REG_META_FIELD(Renderer, clear),
        REG_META_FIELD(Renderer, enable),
        REG_META_FIELD(Renderer, disable),
        REG_META_FIELD(Renderer, set_texture2d),
        REG_META_FIELD(Renderer, set_framebuffer),
        REG_META_FIELD(Renderer, set_program),
        REG_META_FIELD(Renderer, set_viewport),
        REG_META_FIELD(Renderer, set_scissor),
        REG_META_FIELD(Renderer, set_blend_mode),
        REG_META_FIELD(Renderer, draw),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}

int luaopen_renderer(lua_State* L) {
    lua_newtable(L);
    // Renderer meta
    l_Renderer_meta(L);
    lua_setfield(L, -2, "Renderer");
    // RenderList meta
    l_RenderList_meta(L);
    lua_setfield(L, -2, "RenderList");

    // Effect2D
    l_Effect2D_open_meta(L);
    lua_setfield(L, -2, EFFECT2D_CLASS);
    // Texture2D
    l_Texture2D_open_meta(L);
    lua_setfield(L, -2, TEXTURE2D_CLASS);
    // Canvas
    l_Canvas_open_meta(L);
    lua_setfield(L, -2, CANVAS_CLASS);

    // RenderBatch2D
    l_RenderBatch2D_meta(L);
    lua_setfield(L, -2, "RenderBatch2D");

    const luaL_Reg reg[] = {
        // {"create_batch2D", l_renderer_create_Batch2D},
        {"create", l_renderer_create},
        {"create_canvas", l_Canvas_create},
        {"create_texture", l_Texture2D_create},
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
