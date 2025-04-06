#include "selene_renderer.h"

static const char* blend_modes[] = {"alpha", "additive", "subtractive", "multiply", NULL};

const char* projection_modes[] = {"orthographic", "frustum", "perspective", NULL};

const char* clear_masks[] = {"color", "depth", "stencil", NULL};
const int clear_masks_values[] = {GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT };

const char* enable_attribs[] = { "blend", "depth test", "scissor test", "cull face", NULL };
const int enable_attribs_values[] = { GL_BLEND, GL_DEPTH_TEST, GL_SCISSOR_TEST, GL_CULL_FACE };

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
extern int l_Texture2D_load(lua_State* L);

extern int l_Font_open_meta(lua_State* L);
extern int l_Font_8x8(lua_State* L);
extern int l_Font_load(lua_State* L);

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
    // fprintf(stdout, "render list: %p\n", r->list_ptr);
    r->l_render_list_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    r->l_gl_context_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, 1);
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

static int l_Renderer__set_clear_color(lua_State* L) {
    CHECK_META(Renderer);
    self->clear_color[0] = 0.f;
    self->clear_color[1] = 0.f;
    self->clear_color[2] = 0.f;
    self->clear_color[3] = 1.f;
    for (int i = 2; i <= lua_gettop(L); i++) {
        self->clear_color[i-2] = (float)luaL_checknumber(L, i);
    }
    return 0;
}

static int l_Renderer__clear(lua_State* L) {
    CHECK_META(Renderer);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_CLEAR;
    int opt = luaL_checkoption(L, arg++, "color", clear_masks);
    int mask = clear_masks_values[opt];
    if (lua_gettop(L) >= arg) {
        for (int i = arg; i <= lua_gettop(L); i++) {
            opt = luaL_checkoption(L, i, NULL, clear_masks);
            if (opt != -1) mask |= clear_masks_values[opt];
        }
    }
    rc.clear.mask = mask;
    rc.clear.color[0] = self->clear_color[0];
    rc.clear.color[1] = self->clear_color[1];
    rc.clear.color[2] = self->clear_color[2];
    rc.clear.color[3] = self->clear_color[3];
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

int l_Renderer__set_vao(lua_State* L) {
    CHECK_META(Renderer);
    Uint32 handle = (int)luaL_optinteger(L, arg++, 0); 
    if (handle != self->current_vao_id) {
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_VERTEX_ARRAY;
        rc.vao.handle = handle;
        RENDERLIST_PUSH(self->list_ptr, &rc);
        self->current_vao_id = (int)handle;
    }
    return 0;
}

static int l_Renderer__set_texture(lua_State* L) {
    CHECK_META(Renderer);
    int target = luaL_checkoption(L, arg++, "tex2d", texture_targets);
    Uint32 handle = (Uint32)luaL_optinteger(L, arg++, 0);
    Uint32 slot = (Uint32)luaL_optinteger(L, arg++, 0);
    if (handle != self->current_tex2d_id) {
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_TEXTURE;
        rc.texture.slot = slot;
        rc.texture.target = texture_targets_values[target];
        rc.texture.handle = handle;
        RENDERLIST_PUSH(self->list_ptr, &rc);
        self->current_tex2d_id = (int)handle;
    }

    return 0;
}

static int l_Renderer__set_texture2d(lua_State* L) {
    CHECK_META(Renderer);
    Uint32 handle = (Uint32)luaL_optinteger(L, arg++, 0);
    Uint32 slot = (Uint32)luaL_optinteger(L, arg++, 0);
    if (handle != self->current_tex2d_id) {
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_TEXTURE;
        rc.texture.slot = slot;
        rc.texture.target = GL_TEXTURE_2D;
        rc.texture.handle = handle;
        RENDERLIST_PUSH(self->list_ptr, &rc);
        self->current_tex2d_id = (int)handle;
    }
    return 0;
}

static int l_Renderer__set_framebuffer(lua_State* L) {
    CHECK_META(Renderer);
    Uint32 handle = (Uint32)luaL_optinteger(L, arg++, 0);
    if (handle != self->current_fbo_id) {
        struct RenderCommand rc;
        /* Set target */
        rc.type = RENDER_COMMAND_SET_FRAMEBUFFER;
        rc.target.target = GL_FRAMEBUFFER;
        rc.target.handle = handle;
        RENDERLIST_PUSH(self->list_ptr, &rc);
        self->current_fbo_id = (int)handle;
    }
    return 0;
}

static int l_Renderer__set_program(lua_State* L) {
    CHECK_META(Renderer);
    Uint32 handle = (Uint32)luaL_optinteger(L, arg++, 0);
    if (handle != self->current_program_id) {
        struct RenderCommand rc;
        rc.type = RENDER_COMMAND_SET_PROGRAM;
        rc.program.handle = handle;
        RENDERLIST_PUSH(self->list_ptr, &rc);
        self->current_program_id = (int)handle;
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
    rc.type = RENDER_COMMAND_SET_SCISSOR;
    CHECK_INTEGER(left);
    CHECK_INTEGER(top);
    CHECK_INTEGER(right);
    CHECK_INTEGER(bottom);
    rc.clip.x = left;
    rc.clip.y = top;
    rc.clip.width = right;
    rc.clip.height = bottom;
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

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
        default:
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
    rc.draw.start = (int)luaL_checkinteger(L, arg++);
    rc.draw.count = (int)luaL_checkinteger(L, arg++);
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__draw_instanced(lua_State* L) {
    CHECK_META(Renderer);
    int opt = luaL_checkoption(L, arg++, "triangles", draw_modes);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_DRAW_VERTEX_INSTANCED;
    rc.instanced.mode = draw_modes_values[opt];
    rc.instanced.start = (int)luaL_checkinteger(L, arg++);
    rc.instanced.count = (int)luaL_checkinteger(L, arg++);
    rc.instanced.instance_count = (int)luaL_checkinteger(L, arg++);
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__send_float(lua_State* L) {
    CHECK_META(Renderer);
    const char* name = luaL_checkstring(L, arg++);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_FLOAT_UNIFORM;
    rc.uniform.location = glGetUniformLocation(self->current_program_id, name);
    rc.uniform.f = (float)luaL_checknumber(L, arg);
    RENDERLIST_PUSH(self->list_ptr, &rc);
    return 0;
}

static int l_Renderer__send_matrix(lua_State* L) {
    CHECK_META(Renderer);
    const char* name = luaL_checkstring(L, arg++);
    struct RenderCommand rc;
    rc.type = RENDER_COMMAND_MATRIX_UNIFORM;
    rc.uniform.location = glGetUniformLocation(self->current_program_id, name);
    if (lua_istable(L, arg)) {
        if (lua_rawlen(L, arg) < 16)
            return luaL_argerror(L, arg, "table must have at least 16 elements");
        float* m = (float*)rc.uniform.m;
        for (int i = 0; i < 16; i++) {
            lua_rawgeti(L, arg, i+1);
            m[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    } else {
        float* m = (float*)rc.uniform.m;
        for (int i = 0; i < 16; i++) {
            if (i % 5 == 0) m[i] = 1.f;
            else m[i] = 0.f;
        }
    }
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
        // set states
        REG_META_FIELD(Renderer, set_vao),
        // REG_META_FIELD(Renderer, set_buffer),
        REG_META_FIELD(Renderer, set_texture2d),
        REG_META_FIELD(Renderer, set_framebuffer),
        REG_META_FIELD(Renderer, set_program),
        REG_META_FIELD(Renderer, set_viewport),
        REG_META_FIELD(Renderer, set_scissor),
        REG_META_FIELD(Renderer, set_blend_mode),
        // uniforms
        REG_META_FIELD(Renderer, send_float),
        REG_META_FIELD(Renderer, send_matrix),
        // draw
        REG_META_FIELD(Renderer, draw),
        REG_META_FIELD(Renderer, draw_instanced),
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}

static int l_selene_renderer_load_image(lua_State* L) {
    INIT_ARG();
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

    l_Font_open_meta(L);
    lua_setfield(L, -2, "Font");

    // RenderBatch2D
    l_RenderBatch2D_meta(L);
    lua_setfield(L, -2, "RenderBatch2D");

    const luaL_Reg reg[] = {
        // {"create_batch2D", l_renderer_create_Batch2D},
        // {"create", l_renderer_create},
        {"create_canvas", l_Canvas_create},
        {"create_texture", l_Texture2D_create},
        {"load_texture", l_Texture2D_load},
        {"font8x8", l_Font_8x8},
        {"load_font", l_Font_load},
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
