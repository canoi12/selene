#include "modules/renderer.h"
#include "modules/window.h"

static inline void s_push_commands(selene_Renderer* r, int count, struct RenderCommand* cmd) {
    if (r->command_offset + count > r->command_count) {
        r->command_count *= 2;
        r->command_pool = (struct RenderCommand*)realloc(r->command_pool, sizeof(*cmd) * r->command_count);
    }
    memcpy(r->command_pool + r->command_offset, cmd, sizeof(*cmd) * count);
    r->command_offset += count;
}
#define PUSH_COMMAND(r, cmd) s_push_commands(r, 1, cmd)

static int l_Renderer__get_backend(lua_State* L) {
    CHECK_META(selene_Renderer);
    switch (self->backend) {
        case SELENE_RENDERER_OPENGL: lua_pushstring(L, "opengl"); break;
        case SELENE_RENDERER_DIRECTX11: lua_pushstring(L, "dx11"); break;
        case SELENE_RENDERER_VULKAN: lua_pushstring(L, "vulkan"); break;
        default: lua_pushstring(L, "unknown");
    }
    return 1;
}


int l_Renderer__destroy(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->destroy(L);
}

/**
 * Clear
 */

int l_Renderer__clear_color(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_CLEAR_COLOR;
    int top = lua_gettop(L);
    vec4 c = { 0, 0, 0, 1 };
    memcpy(cmd.clear.color, c, sizeof(vec4));
    for (int i = 0; i < (top - 1); i++) cmd.clear.color[i] = (float)luaL_checknumber(L, i + 2);
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__clear_depth(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_NUMBER(float, depth);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_CLEAR_DEPTH;
    cmd.clear.depth = depth;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

/**
 * Pipeline
 */

int l_Renderer__create_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->create_pipeline(L);
}

int l_Renderer__destroy_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->destroy_pipeline(L);
}

static int l_Renderer__set_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderPipeline, pipeline);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_PIPELINE;
    cmd.pipeline = pipeline;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

/**
 * Buffer
 */
int l_Renderer__create_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->create_buffer(L);
}

int l_Renderer__destroy_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->destroy_buffer(L);
}

int l_Renderer__set_vertex_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_VERTEX_BUFFER;
    cmd.buffer.ptr = buffer;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__set_index_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_INDEX_BUFFER;
    cmd.buffer.ptr = buffer;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__set_uniform_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_UNIFORM_BUFFER;
    cmd.buffer.ptr = buffer;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__send_buffer_data(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->send_buffer_data(L);
}

int l_Renderer__send_buffer_ortho(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(offset);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, n);
    CHECK_NUMBER(float, f);
    mat4 matrix;
    glm_mat4_identity(matrix);
    glm_ortho(left, right, bottom, top, n, f, matrix);
    lua_pushcfunction(L, self->send_buffer_data);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_pushinteger(L, sizeof(matrix));
    lua_pushlightuserdata(L, matrix);
    lua_call(L, 4, 0);
    return 0;
}

/**
 * Texture
 */

int l_Renderer__create_texture2d(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->create_texture2d(L);
}

#include "stb_image.h"
int l_Renderer__load_texture2d(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_STRING(filename);
#if defined(SELENE_USE_SDL3)
    SDL_IOStream *rw = SDL_IOFromFile(filename, "rb");
    size_t size = SDL_GetIOSize(rw);
    void* data = malloc(size);
    SDL_ReadIO(rw, data, size);
    SDL_CloseIO(rw);
#else
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
    if (!rw) {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to open file %s\n", filename);
        return 2;
    }
    size_t size = SDL_RWsize(rw);
    void* data = malloc(size);
    SDL_RWread(rw, data, 1, size);
    SDL_RWclose(rw);
#endif
    int w, h, comp;
    int req_comp = STBI_rgb_alpha;
    stbi_uc* pixels = stbi_load_from_memory((stbi_uc const*)data, size, &w, &h, &comp, req_comp);
    free(data);
    
    lua_pushcfunction(L, self->create_texture2d);
    lua_pushvalue(L, 1);
    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    lua_pushstring(L, "rgba");
    lua_pushlightuserdata(L, pixels);
    lua_call(L, 5, 1);
    stbi_image_free(pixels);
    return 1;
}

int l_Renderer__create_depth_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->create_depth_texture(L);
}

int l_Renderer__destroy_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->destroy_texture(L);
}

static int l_Renderer__set_texture(lua_State* L) {
    CHECK_META(selene_Renderer);
    TEST_UDATA(selene_Texture2D, tex);
    OPT_INTEGER(slot, 0);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_TEXTURE;
    cmd.texture.target = GL_TEXTURE_2D;
    cmd.texture.ptr = (void*)tex;
    cmd.texture.slot = slot;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

/**
 * Render Target
 */

int l_Renderer__create_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->create_render_target(L);
}

int l_Renderer__destroy_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->destroy_render_target(L);
}

static int l_Renderer__set_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    TEST_UDATA(selene_RenderTarget, target);
    struct RenderCommand cmd;
    cmd.target.depth = 0;
    cmd.type = RENDER_COMMAND_SET_RENDER_TARGET;
    if (lua_type(L, arg) == LUA_TBOOLEAN) cmd.target.depth = lua_toboolean(L, arg++);
    if (!target) {
        cmd.target.depth = self->default_target.depth != NULL;
    }
    cmd.target.ptr = target;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

/**
 * Shader
 */

int l_Renderer__create_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->create_shader(L);
}

int l_Renderer__load_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    const char* filename = luaL_checkstring(L, 3);
#if defined(SELENE_USE_SDL3)
    SDL_IOStream *rw = SDL_IOFromFile(filename, "rb");
    size_t size = SDL_GetIOSize(rw);
    char* data = malloc(size+1);
    SDL_ReadIO(rw, data, size);
    SDL_CloseIO(rw);
#else
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
    size_t size = SDL_RWsize(rw);
    char* data = malloc(size+1);
    SDL_RWread(rw, data, 1, size);
    SDL_RWclose(rw);
#endif
    data[size] = '\0';
    
    lua_pushcfunction(L, self->create_shader);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_pushlstring(L, data, size);
    lua_call(L, 3, 1);
    free(data);

    return 1;
}

int l_Renderer__destroy_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->destroy_shader(L);
}

/**
 * Draw and flush
 */

int l_Renderer__set_viewport(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_VIEWPORT;
    cmd.viewport.x = luaL_checkinteger(L, arg++);
    cmd.viewport.y = luaL_checkinteger(L, arg++);
    cmd.viewport.width = luaL_checkinteger(L, arg++);
    cmd.viewport.height = luaL_checkinteger(L, arg++);
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__set_scissor(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_SET_SCISSOR;
    cmd.scissor.x = (int)luaL_checkinteger(L, arg++);
    cmd.scissor.y = (int)luaL_checkinteger(L, arg++);
    cmd.scissor.width = (int)luaL_checkinteger(L, arg++);
    cmd.scissor.height = (int)luaL_checkinteger(L, arg++);
    if (self->backend == SELENE_RENDERER_OPENGL) {
        int width, height;
        SDL_GetWindowSize(self->window_ptr->handle, &width, &height);
        cmd.scissor.y = height - cmd.scissor.y - cmd.scissor.height;

    }
    else if (self->backend == SELENE_RENDERER_DIRECTX11) {
        cmd.scissor.width += cmd.scissor.x;
        cmd.scissor.height += cmd.scissor.y;
    }
#if 0
    if (lua_isnoneornil(L, arg)) cmd.scissor.enabled = 0;
    else if (lua_isinteger(L, arg)) {
        cmd.scissor.enabled = 1;
        cmd.scissor.x = (int)luaL_checkinteger(L, arg++);
        cmd.scissor.y = (int)luaL_checkinteger(L, arg++);
        cmd.scissor.width = (int)luaL_checkinteger(L, arg++);
        cmd.scissor.height = (int)luaL_checkinteger(L, arg++);
    }
    else return luaL_argerror(L, arg, "must be none or (x, y, width, height)");
#endif
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__draw(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "triangles", draw_modes);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_DRAW_VERTEX;
    cmd.draw.mode = opt;
    cmd.draw.start = (int)luaL_checkinteger(L, arg++);
    cmd.draw.count = (int)luaL_checkinteger(L, arg++);
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__flush(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->flush(L);
}

int l_Renderer__present(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->present(L);
}

int l_Renderer_meta(lua_State* L) {
    luaL_newmetatable(L, selene_Renderer_METANAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg _reg[] = {
        REG_META_FIELD(Renderer, get_backend),
        REG_META_FIELD(Renderer, destroy),
        // Flush and draw
        REG_META_FIELD(Renderer, set_viewport),
        REG_META_FIELD(Renderer, set_scissor),
        REG_META_FIELD(Renderer, draw),
        REG_META_FIELD(Renderer, flush),
        REG_META_FIELD(Renderer, present),
        // Clear
        REG_META_FIELD(Renderer, clear_color),
        REG_META_FIELD(Renderer, clear_depth),
        // Pipeline
        REG_META_FIELD(Renderer, create_pipeline),
        REG_META_FIELD(Renderer, destroy_pipeline),
        REG_META_FIELD(Renderer, set_pipeline),
        // Buffer
        REG_META_FIELD(Renderer, create_buffer),
        REG_META_FIELD(Renderer, destroy_buffer),
        REG_META_FIELD(Renderer, set_vertex_buffer),
        REG_META_FIELD(Renderer, set_index_buffer),
        REG_META_FIELD(Renderer, set_uniform_buffer),
        REG_META_FIELD(Renderer, send_buffer_data),
        REG_META_FIELD(Renderer, send_buffer_ortho),
        // Texture2D
        REG_META_FIELD(Renderer, create_texture2d),
        REG_META_FIELD(Renderer, load_texture2d),
        REG_META_FIELD(Renderer, create_depth_texture),
        REG_META_FIELD(Renderer, destroy_texture),
        REG_META_FIELD(Renderer, set_texture),
        // Render Target
        REG_META_FIELD(Renderer, create_render_target),
        REG_META_FIELD(Renderer, destroy_render_target),
        REG_META_FIELD(Renderer, set_render_target),
        // Shader
        REG_META_FIELD(Renderer, create_shader),
        REG_META_FIELD(Renderer, load_shader),
        REG_META_FIELD(Renderer, destroy_shader),
        {NULL, NULL}
    };
    luaL_setfuncs(L, _reg, 0);
#if 0
    const luaL_Reg reg[] = {
            REG_META_FIELD(Renderer, destroy),
            REG_META_FIELD(Renderer, begin),
            REG_META_FIELD(Renderer, finish),
            REG_META_FIELD(Renderer, clear),
            REG_META_FIELD(Renderer, enable),
            REG_META_FIELD(Renderer, disable),
            // set states
            //REG_META_FIELD(Renderer, set_vao),
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
#endif
    return 1;
}
