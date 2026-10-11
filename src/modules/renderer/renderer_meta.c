#include "cglm/mat4.h"
#include "lua.h"
#include "lua_helper.h"
#include "modules/renderer.h"
#include "modules/window.h"
#include "modules/filesystem.h"

int next_texture_id = 0;
int next_pipeline_id = 0;
int next_shader_id = 0;

#define PUSH_COMMAND(r, cmd) renderer_push_command((r), (cmd))

static int l_Renderer__get_backend(lua_State* L) {
    CHECK_META(selene_Renderer);
    switch (self->backend) {
        case SELENE_RENDERER_OPENGL: lua_pushstring(L, "opengl"); break;
        case SELENE_RENDERER_DIRECTX11: lua_pushstring(L, "d3d11"); break;
        case SELENE_RENDERER_DIRECTX12: lua_pushstring(L, "d3d12"); break;
        case SELENE_RENDERER_VULKAN: lua_pushstring(L, "vulkan"); break;
        case SELENE_RENDERER_METAL: lua_pushstring(L, "metal"); break;
        case SELENE_RENDERER_AGC: lua_pushstring(L, "agc"); break;
        case SELENE_RENDERER_GNM: lua_pushstring(L, "gnm"); break;
        default: lua_pushstring(L, "unknown");
    }
    return 1;
}

static int l_Renderer__get_info(lua_State* L) {
    CHECK_META(selene_Renderer);
    lua_newtable(L);
    l_Renderer__get_backend(L);
    lua_setfield(L, -2, "backend");
#ifndef SELENE_NO_OPENGL
    if (self->backend == SELENE_RENDERER_OPENGL) {
        const struct selene_GLCapabilities* caps = &self->gl.caps;
        lua_pushboolean(L, caps->is_gles);
        lua_setfield(L, -2, "es");
        lua_pushinteger(L, caps->major);
        lua_setfield(L, -2, "major");
        lua_pushinteger(L, caps->minor);
        lua_setfield(L, -2, "minor");
        lua_pushboolean(L, caps->has_vao);
        lua_setfield(L, -2, "vao");
        lua_pushboolean(L, caps->has_instancing);
        lua_setfield(L, -2, "instancing");
        lua_pushboolean(L, caps->has_ubo);
        lua_setfield(L, -2, "ubo");
        lua_pushboolean(L, caps->has_blend_equation);
        lua_setfield(L, -2, "blend_equation");
        lua_pushboolean(L, caps->has_depth_texture);
        lua_setfield(L, -2, "depth_texture");
        lua_pushboolean(L, caps->has_bgra);
        lua_setfield(L, -2, "bgra");
        lua_pushboolean(L, caps->has_unpack_flip_y);
        lua_setfield(L, -2, "unpack_flip_y");
    }
#endif
    return 1;
}

static int l_Renderer__get_size(lua_State* L) {
    CHECK_META(selene_Renderer);
    lua_pushinteger(L, self->width);
    lua_pushinteger(L, self->height);
    return 2;
}

static int l_Renderer__set_size(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    self->width = width;
    self->height = height;
    return 0;
}

int l_Renderer__destroy(lua_State* L) {
    CHECK_META(selene_Renderer);
    return self->destroy(L);
}

static int l_Renderer__get_object_id(lua_State* L) {
    CHECK_META(selene_Renderer);
    int id = -1;
    TEST_UDATA(selene_Texture2D, texture);
    if (texture) id = texture->id;
    --arg;
    TEST_UDATA(selene_Shader, shader);
    if (shader) id = shader->id;
    // TEST_UDATA(selene_GpuBuffer, buffer);
    // if (buffer) id = buffer->id;
    // TEST_UDATA(selene_RenderPipeline, pipeline);
    // if (pipeline) id = pipeline->id;
    lua_pushinteger(L, id);
    return 1;
}

static int s_read_ivec4_field(lua_State* L, int tbl, const char* name, int* x, int* y, int* w, int* h) {
    if (lua_getfield(L, tbl, name) != LUA_TTABLE) {
        lua_pop(L, 1);
        return 0;
    }
    lua_rawgeti(L, -1, 1); *x = (int)luaL_optinteger(L, -1, 0); lua_pop(L, 1);
    lua_rawgeti(L, -1, 2); *y = (int)luaL_optinteger(L, -1, 0); lua_pop(L, 1);
    lua_rawgeti(L, -1, 3); *w = (int)luaL_optinteger(L, -1, 0); lua_pop(L, 1);
    lua_rawgeti(L, -1, 4); *h = (int)luaL_optinteger(L, -1, 0); lua_pop(L, 1);
    lua_pop(L, 1);
    return 1;
}

static void s_fill_begin_pass(lua_State* L, int tbl, selene_Renderer* r, struct RenderCommand* cmd) {
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = RENDER_COMMAND_BEGIN_PASS;
    if (!lua_istable(L, tbl)) return;

    if (lua_getfield(L, tbl, "target") != LUA_TNIL) {
        cmd->pass.target = (selene_RenderTarget*)luaL_testudata(L, -1, selene_RenderTarget_METANAME);
    }
    lua_pop(L, 1);

    if (lua_getfield(L, tbl, "clear_color") == LUA_TTABLE) {
        cmd->pass.clear_color = 1;
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, -1, i + 1);
            cmd->pass.color[i] = (float)luaL_optnumber(L, -1, i == 3 ? 1.0 : 0.0);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    if (lua_getfield(L, tbl, "clear_depth") == LUA_TNUMBER) {
        cmd->pass.clear_depth = 1;
        cmd->pass.depth = (float)lua_tonumber(L, -1);
    }
    lua_pop(L, 1);

    cmd->pass.has_viewport = s_read_ivec4_field(
        L, tbl, "viewport",
        &cmd->pass.viewport.x, &cmd->pass.viewport.y,
        &cmd->pass.viewport.width, &cmd->pass.viewport.height);

    if (s_read_ivec4_field(
            L, tbl, "scissor",
            &cmd->pass.scissor.x, &cmd->pass.scissor.y,
            &cmd->pass.scissor.width, &cmd->pass.scissor.height)) {
        cmd->pass.has_scissor = 1;
        cmd->pass.scissor.enabled = 1;
    }
    (void)r;
}

int l_Renderer__begin_frame(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (self->begin_frame) return self->begin_frame(L);
    self->in_frame = 1;
    self->in_pass = 0;
    return 0;
}

int l_Renderer__end_frame(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (self->end_frame) return self->end_frame(L);
    self->in_pass = 0;
    self->in_frame = 0;
    if (self->flush && self->command_offset > 0) return self->flush(L);
    return 0;
}

int l_Renderer__begin_pass(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (self->begin_pass) return self->begin_pass(L);
    struct RenderCommand cmd;
    s_fill_begin_pass(L, arg, self, &cmd);
    self->in_pass = 1;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__end_pass(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (self->end_pass) return self->end_pass(L);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_END_PASS;
    self->in_pass = 0;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__create_frame_data(lua_State* L) {
    CHECK_META(selene_Renderer);
    selene_FrameData* frame_data = lua_newuserdata(L, sizeof(selene_FrameData));
    luaL_setmetatable(L, selene_FrameData_METANAME);
    glm_mat4_identity(frame_data->projection);
    glm_mat4_identity(frame_data->view);
    frame_data->time = 0.f;
    frame_data->delta_time = 0.f;
    frame_data->screen_width = 0;
    frame_data->screen_height = 0;
    return 1;
}

int l_FrameData__ortho(lua_State* L) {
    CHECK_META(selene_FrameData);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near_plane);
    CHECK_NUMBER(float, far_plane);
    glm_ortho(left, right, bottom, top, near_plane, far_plane, self->projection);
    return 0;
}

int l_FrameData__perspective(lua_State* L) {
    CHECK_META(selene_FrameData);
    CHECK_NUMBER(float, fovy);
    CHECK_NUMBER(float, aspect);
    CHECK_NUMBER(float, near_plane);
    CHECK_NUMBER(float, far_plane);
    glm_perspective(fovy, aspect, near_plane, far_plane, self->projection);
    return 0;
}

int l_FrameData__lookat(lua_State* L) {
    CHECK_META(selene_FrameData);
    CHECK_NUMBER(float, eyex);
    CHECK_NUMBER(float, eyey);
    CHECK_NUMBER(float, eyez);
    CHECK_NUMBER(float, centerx);
    CHECK_NUMBER(float, centery);
    CHECK_NUMBER(float, centerz);
    CHECK_NUMBER(float, upx);
    CHECK_NUMBER(float, upy);
    CHECK_NUMBER(float, upz);
    glm_lookat((vec3){ eyex, eyey, eyez }, (vec3){ centerx, centery, centerz }, (vec3){ upx, upy, upz }, self->view);
    return 0;
}

int l_FrameData__time(lua_State* L) {
    CHECK_META(selene_FrameData);
    CHECK_NUMBER(float, time);
    self->time = time;
    return 0;
}

int l_FrameData__delta_time(lua_State* L) {
    CHECK_META(selene_FrameData);
    CHECK_NUMBER(float, delta_time);
    self->delta_time = delta_time;
    return 0;
}

int l_FrameData__screen_width(lua_State* L) {
    CHECK_META(selene_FrameData);
    CHECK_NUMBER(int, screen_width);
    self->screen_width = screen_width;
    return 0;
}

int l_FrameData__screen_height(lua_State* L) {
    CHECK_META(selene_FrameData);
    CHECK_NUMBER(int, screen_height);
    self->screen_height = screen_height;
    return 0;
}

int l_FrameData_meta(lua_State* L) {
    luaL_newmetatable(L, selene_FrameData_METANAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg _reg[] = {
        {"ortho", l_FrameData__ortho},
        {"perspective", l_FrameData__perspective},
        {"lookat", l_FrameData__lookat},
        {"time", l_FrameData__time},
        {"delta_time", l_FrameData__delta_time},
        {"screen_width", l_FrameData__screen_width},
        {"screen_height", l_FrameData__screen_height},
        {NULL, NULL}
    };
    luaL_setfuncs(L, _reg, 0);
    lua_pop(L, 1);
    return 0;
}

/**
 * Clear
 */

int l_Renderer__clear_color(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_CLEAR_COLOR;
    cmd.clear.color[3] = 1.f;
    int top = lua_gettop(L);
    for (int i = 0; i < (top - 1) && i < 4; i++)
        cmd.clear.color[i] = (float)luaL_checknumber(L, i + 2);
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__clear_depth(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_NUMBER(float, depth);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
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
    memset(&cmd, 0, sizeof(cmd));
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

static int s_parse_buffer_slot(lua_State* L, int* arg, selene_GpuBuffer** out_buf, int* out_slot) {
    int slot = 0;
    selene_GpuBuffer* buffer = NULL;
    if (lua_type(L, *arg) == LUA_TNUMBER) {
        slot = (int)luaL_checkinteger(L, (*arg)++);
        buffer = (selene_GpuBuffer*)luaL_checkudata(L, (*arg)++, selene_GpuBuffer_METANAME);
    } else {
        buffer = (selene_GpuBuffer*)luaL_checkudata(L, (*arg)++, selene_GpuBuffer_METANAME);
        if (lua_type(L, *arg) == LUA_TNUMBER) slot = (int)luaL_optinteger(L, (*arg)++, 0);
    }
    *out_buf = buffer;
    *out_slot = slot;
    return 0;
}

int l_Renderer__set_vertex_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    selene_GpuBuffer* buffer = NULL;
    int slot = 0;
    s_parse_buffer_slot(L, &arg, &buffer, &slot);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_SET_VERTEX_BUFFER;
    cmd.buffer.ptr = buffer;
    cmd.buffer.slot = slot;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__set_index_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_SET_INDEX_BUFFER;
    cmd.buffer.ptr = buffer;
    cmd.buffer.index_type = buffer->index_type ? buffer->index_type : SELENE_INDEX_UINT32;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__set_uniform_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    selene_GpuBuffer* buffer = NULL;
    int slot = 0;
    s_parse_buffer_slot(L, &arg, &buffer, &slot);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_SET_UNIFORM_BUFFER;
    cmd.buffer.ptr = buffer;
    cmd.buffer.slot = slot;
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
    renderer_clip_ortho(self, left, right, bottom, top, n, f, matrix);
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
    int virt = get_virt_path(L, filename);
    if (virt > 0) filename = lua_tostring(L, -1);
    lua_pop(L, 1);
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
    OPT_INTEGER(binding, self->backend == SELENE_RENDERER_VULKAN ? 1 : 0);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_SET_TEXTURE;
    cmd.texture.target = SELENE_TEXTURE_2D;
    cmd.texture.ptr = (void*)tex;
    cmd.texture.slot = binding;
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
    memset(&cmd, 0, sizeof(cmd));
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
    int virt = get_virt_path(L, filename);
    if (virt > 0) filename = lua_tostring(L, -1);
    lua_pop(L, 1);
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
    if (!rw) {
        DEBUG_ERROR("[renderer] failed to load shader: %s\n", filename);
        return luaL_error(L, "failed to load shader: %s", filename);
    }
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

int l_Renderer__update_uniform(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(offset);
    CHECK_INTEGER(size);
    CHECK_LUDATA(void, data);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_UPDATE_UNIFORM_BUFFER;
    // cmd.uniform.binding = 0;
    cmd.uniform.buffer = buffer;
    cmd.uniform.offset = offset;
    cmd.uniform.size = size;
    memcpy(cmd.uniform.m, data, size);
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__update_uniform_matrix(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(offset);
    int size = sizeof(mat4);
    float m[16];
    for (int i = 0; i < 16; i++) {
        m[i] = luaL_checknumber(L, arg + i);
    }
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_UPDATE_UNIFORM_BUFFER;
    cmd.uniform.buffer = buffer;
    cmd.uniform.offset = offset;
    cmd.uniform.size = size;
    memcpy(cmd.uniform.m, m, sizeof(m));
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__update_uniform_ortho(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_INTEGER(offset);
    mat4 m;
    glm_mat4_identity(m);
    CHECK_NUMBER(float, left);
    CHECK_NUMBER(float, right);
    CHECK_NUMBER(float, bottom);
    CHECK_NUMBER(float, top);
    CHECK_NUMBER(float, near_plane);
    CHECK_NUMBER(float, far_plane);
    glm_ortho(left, right, bottom, top, near_plane, far_plane, m);
    struct RenderCommand cmd;
    cmd.type = RENDER_COMMAND_UPDATE_UNIFORM_BUFFER;
    cmd.uniform.buffer = buffer;
    cmd.uniform.offset = offset;
    cmd.uniform.size = sizeof(m);
    memcpy(cmd.uniform.m, m, sizeof(m));
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__update_frame_data(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    CHECK_UDATA(selene_FrameData, frame);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_UPDATE_UNIFORM_BUFFER;
    cmd.uniform.buffer = buffer;
    cmd.uniform.offset = 0;
    cmd.uniform.size = sizeof(selene_FrameData);
    memcpy(&cmd.uniform.frame, frame, sizeof(selene_FrameData));
    PUSH_COMMAND(self, &cmd);
    return 0;
}

/**
 * Draw and flush
 */

int l_Renderer__set_viewport(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_SET_VIEWPORT;
    cmd.viewport.x = (int)luaL_checkinteger(L, arg++);
    cmd.viewport.y = (int)luaL_checkinteger(L, arg++);
    cmd.viewport.width = (int)luaL_checkinteger(L, arg++);
    cmd.viewport.height = (int)luaL_checkinteger(L, arg++);
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__set_scissor(lua_State* L) {
    CHECK_META(selene_Renderer);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_SET_SCISSOR;
    if (lua_isnoneornil(L, arg)) {
        cmd.scissor.enabled = 0;
    } else {
        cmd.scissor.enabled = 1;
        cmd.scissor.x = (int)luaL_checkinteger(L, arg++);
        cmd.scissor.y = (int)luaL_checkinteger(L, arg++);
        cmd.scissor.width = (int)luaL_checkinteger(L, arg++);
        cmd.scissor.height = (int)luaL_checkinteger(L, arg++);
    }
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__draw(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "triangles", draw_modes);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_DRAW;
    cmd.draw.topology = opt;
    cmd.draw.start = (int)luaL_checkinteger(L, arg++);
    cmd.draw.count = (int)luaL_checkinteger(L, arg++);
    cmd.draw.instance_count = (int)luaL_optinteger(L, arg, 1);
    if (cmd.draw.instance_count < 1) cmd.draw.instance_count = 1;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__draw_indexed(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "triangles", draw_modes);
    struct RenderCommand cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.type = RENDER_COMMAND_DRAW_INDEXED;
    cmd.draw.topology = opt;
    cmd.draw.start = (int)luaL_checkinteger(L, arg++);
    cmd.draw.count = (int)luaL_checkinteger(L, arg++);
    cmd.draw.instance_count = (int)luaL_optinteger(L, arg, 1);
    if (cmd.draw.instance_count < 1) cmd.draw.instance_count = 1;
    PUSH_COMMAND(self, &cmd);
    return 0;
}

int l_Renderer__flush(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (!self->flush) return 0;
    return self->flush(L);
}

int l_Renderer__present(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (!self->present) return 0;
    return self->present(L);
}

int l_Renderer_meta(lua_State* L) {
    luaL_newmetatable(L, selene_Renderer_METANAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg _reg[] = {
        REG_META_FIELD(Renderer, get_backend),
        REG_META_FIELD(Renderer, get_info),
        REG_META_FIELD(Renderer, get_size),
        REG_META_FIELD(Renderer, set_size),
        REG_META_FIELD(Renderer, destroy),
        REG_META_FIELD(Renderer, get_object_id),
        REG_META_FIELD(Renderer, begin_frame),
        REG_META_FIELD(Renderer, end_frame),
        REG_META_FIELD(Renderer, begin_pass),
        REG_META_FIELD(Renderer, end_pass),
        REG_META_FIELD(Renderer, set_viewport),
        REG_META_FIELD(Renderer, set_scissor),
        REG_META_FIELD(Renderer, draw),
        REG_META_FIELD(Renderer, draw_indexed),
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
        REG_META_FIELD(Renderer, update_uniform_ortho),
        // Frame data
        REG_META_FIELD(Renderer, create_frame_data),
        REG_META_FIELD(Renderer, update_frame_data),
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
