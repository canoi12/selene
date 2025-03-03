#include "renderer.h"

const char* pixel_formats[] = { "rgb", "rgba", NULL };
const int pixel_formats_values[] = { GL_RGB, GL_RGBA };

const char* texture_filters[] = { "nearest", "linear", NULL };
const int texture_filters_values[] = { GL_NEAREST, GL_LINEAR };

const char* draw_modes[] = {"lines", "triangles", NULL};
const int draw_modes_values[] = {GL_LINES, GL_TRIANGLES};

extern int l_Effect2D_open_meta(lua_State* L);
extern int l_Texture2D_open_meta(lua_State* L);

extern int l_Canvas_open_meta(lua_State* L);
extern int l_Canvas_create(lua_State* L);

extern int l_RenderBatch2D_meta(lua_State* L);
extern int l_RenderBatch2D_create(lua_State* L);

extern int l_renderer_create_Batch2D(lua_State* L);
extern int l_renderer_create_Render3D(lua_State* L);

static void s_renderer_push_command(Renderer* r, struct RenderCommand* rc) {
    struct RenderCommandPool* pool = r->pool;
    if (pool->current >= 512) {
        if (!pool->next) {
            struct RenderCommandPool* cmd = (struct RenderCommandPool*)malloc(sizeof(struct RenderCommandPool));
            cmd->prev = pool;
            cmd->next = NULL;
            cmd->current = 0;
            pool->next = cmd;
        }
        r->pool = pool->next;
        pool = pool->next;
    }
    memcpy(&(pool->commands[pool->current]), rc, sizeof(*rc));
    pool->current++;
}

static struct RenderCommand* s_renderer_pop_command(Renderer* r) {
    if (r->pool->current <= 0) {
        if (!r->pool->prev) return NULL;
        r->pool = r->pool->prev;
    }
    r->pool->current--;
    return r->pool->commands + r->pool->current;
}

static void s_renderer_clear_commands(Renderer* r) {
    r->pool = &r->root;
    r->pool->current = 0;
}

static void s_renderer_call_commands(Renderer* r) {
    struct RenderCommandPool* pool = &r->root;
    int it = 0;
    // fprintf(stdout, "render command count: %d\n", pool->current);
    Uint32 curr_program = 0;
    while (pool) {
        struct RenderCommand* rc = pool->commands + it;
        switch (rc->type) {
            case RENDER_COMMAND_CLEAR: {
                float* c = rc->clear.color;
                glClearColor(c[0], c[1], c[2], c[3]);
                // fprintf(stdout, "c: %f %f %f %f\n", c[0], c[1], c[2], c[3]);
                glClear(rc->clear.mask);
            }
            break;
            case RENDER_COMMAND_ENABLE:
                glEnable(rc->enable.attrib);
                break;
            case RENDER_COMMAND_DISABLE:
                glDisable(rc->disable.attrib);
                break;
            case RENDER_COMMAND_SET_VIEWPORT:
                glViewport(rc->viewport.x, rc->viewport.y, rc->viewport.width, rc->viewport.height);
                break;
            case RENDER_COMMAND_SET_TEXTURE:
                glActiveTexture(GL_TEXTURE0+rc->texture.slot);
                glBindTexture(rc->texture.target, rc->texture.handle);
                break;
            case RENDER_COMMAND_SET_TARGET:
                glBindFramebuffer(rc->target.target, rc->target.handle);
                break;
            case RENDER_COMMAND_SET_EFFECT:
                glUseProgram(rc->effect.handle);
                curr_program = rc->effect.handle;
                break;
            case RENDER_COMMAND_SET_PROJECTION:
            case RENDER_COMMAND_SET_VIEW:
            case RENDER_COMMAND_MATRIX_UNIFORM: {
                glUniformMatrix4fv(rc->uniform.location, 1, GL_FALSE, rc->uniform.m[0]);
            }
            break;
            case RENDER_COMMAND_DRAW_VERTEX: {
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glBindVertexArray(rc->draw.vao);
        #else
                glBindBuffer(GL_ARRAY_BUFFER, rc->draw.vao);
        #endif
                // fprintf(stdout, "draw_vertex: %d %d %d\n", rc->draw.mode, rc->draw.start, rc->draw.count);
                glDrawArrays(rc->draw.mode, rc->draw.start, rc->draw.count);
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glBindVertexArray(0);
        #else
                glBindBuffer(GL_ARRAY_BUFFER, 0);
        #endif
            }
            break;
            case RENDER_COMMAND_DRAW_INDEX: {
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glBindVertexArray(rc->draw.vao);
        #else
                glBindBuffer(GL_ARRAY_BUFFER, rc->draw.vao);
        #endif
                glDrawElements(rc->draw.mode, rc->draw.count, GL_UNSIGNED_INT, NULL);
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glBindVertexArray(0);
        #else
                glBindBuffer(GL_ARRAY_BUFFER, 0);
        #endif
            }
            break;
            case RENDER_COMMAND_DRAW_VERTEX_INSTANCED: {
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glBindVertexArray(rc->instanced.vao);
                glDrawArraysInstanced(rc->instanced.mode, rc->instanced.start, rc->instanced.count, rc->instanced.instance_count);
                glBindVertexArray(0);
        #endif
            }
            break;
            case RENDER_COMMAND_DRAW_INDEX_INSTANCED: {
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glBindVertexArray(rc->instanced.vao);
                glDrawElementsInstanced(rc->instanced.mode, rc->instanced.count, GL_UNSIGNED_INT, NULL, rc->instanced.instance_count);
                glBindVertexArray(0);
        #endif
            }
            break;
            default:
        }
        it++;
        if (it >= pool->current) {
            if (pool->next) {
                pool = pool->next;
                it = 0;
            } else break;
        }
    }
}

static void s_renderer_present(Renderer* r, lua_State* L) {
    // fprintf(stdout, "render present\n");
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_window_ref);
    SDL_Window **window = (SDL_Window **)lua_touserdata(L, -1);
    lua_pop(L, 1);
    SDL_GL_SwapWindow(*window);
}

int g_init_renderer(lua_State* L, Renderer* r, SDL_Window* win) {
    if (!r) {
        lua_pushstring(L, "empty renderer");
        return -1;
    }
    memset(r, 0, sizeof(*r));
    r->pool = &r->root;
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
    return 0;
}

int luaopen_renderer(lua_State* L) {
    lua_newtable(L);
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

    luaL_newmetatable(L, "Renderer");
    lua_setfield(L, -2, "Renderer");

    const luaL_Reg reg[] = {
        // {"create_batch2D", l_renderer_create_Batch2D},
        {"create_canvas", l_Canvas_create},
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
