#include "renderer.h"

static int l_default_RenderList_clear(lua_State* L) {
    struct RenderList* list = (struct RenderList*)lua_touserdata(L, 1);
    list->block_index = 1;
    lua_rawgeti(L, LUA_REGISTRYINDEX, list->l_render_blocks_table_ref);
    lua_rawgeti(L, -1, list->block_index);
    list->block_ptr = (struct RenderCommandBlock*)lua_touserdata(L, -1);
    lua_pop(L, 2);
    list->block_ptr->current = 0;
    return 0;
}

static int l_default_RenderList_push(lua_State* L) {
    struct RenderList* list = (struct RenderList*)lua_touserdata(L, 1);
    struct RenderCommand* cmd = (struct RenderCommand*)lua_touserdata(L, 2);
    struct RenderCommandBlock* block = list->block_ptr;
    if (block->current >= 512) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, list->l_render_blocks_table_ref);
        int blk_tbl = lua_gettop(L);
        list->block_index++;
        lua_rawgeti(L, -1, list->block_index);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            fprintf(stdout, "Expanding commands block");
            struct RenderCommandBlock* new_block = (struct RenderCommandBlock*)lua_newuserdata(L, sizeof(*new_block));
            new_block->current = 0;
            lua_pushvalue(L, -1);
            lua_rawseti(L, blk_tbl, list->block_index);
        }
        list->block_ptr = (struct RenderCommandBlock*)lua_touserdata(L, -1);
        lua_pop(L, 2);
    }
    memcpy(&(block->commands[block->current]), cmd, sizeof(*cmd));
    block->current++;
    return 0;
}

static int l_default_RenderList_pop(lua_State* L) {
    struct RenderList* list = (struct RenderList*)lua_touserdata(L, 1);
    if (list->block_ptr->current <= 0) {
        if (list->block_index == 1) return 0;
        list->block_index--;
        lua_rawgeti(L, LUA_REGISTRYINDEX, list->l_render_blocks_table_ref);
        int blk_tbl = lua_gettop(L);
    }
    list->block_ptr->current--;
    lua_pushlightuserdata(L, list->block_ptr->commands + list->block_ptr->current);
    return 1;
}

static int l_default_RenderList_call(lua_State* L) {
    struct RenderList* list = (struct RenderList*)lua_touserdata(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, list->l_render_blocks_table_ref);
    lua_rawgeti(L, -1, 1);
    struct RenderCommandBlock* block = (struct RenderCommandBlock*)lua_touserdata(L, -1);
    lua_pop(L, 2);
    int it = 0;
    int block_index = 1;
    // fprintf(stdout, "render command count: %d\n", pool->current);
    Uint32 curr_program = 0;
    while (block) {
        struct RenderCommand* rc = block->commands + it;
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
            case RENDER_COMMAND_SET_VERTEX_ARRAY:
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glBindVertexArray(rc->vao.handle);
        #endif
                break;
            case RENDER_COMMAND_SET_BUFFER:
                glBindBuffer(rc->buffer.target, rc->buffer.handle);
                break;
            case RENDER_COMMAND_SET_TEXTURE:
                glActiveTexture(GL_TEXTURE0+rc->texture.slot);
                glBindTexture(rc->texture.target, rc->texture.handle);
                break;
            case RENDER_COMMAND_SET_PROGRAM:
                glUseProgram(rc->program.handle);
                curr_program = rc->program.handle;
                break;
            case RENDER_COMMAND_SET_FRAMEBUFFER:
                glBindFramebuffer(rc->target.target, rc->target.handle);
                break;
            case RENDER_COMMAND_SET_RENDERBUFFER:
                glBindRenderbuffer(rc->target.target, rc->target.handle);
                break;
            case RENDER_COMMAND_ENABLE_CLIP_RECT:
                glEnable(GL_SCISSOR_TEST);
                glScissor(rc->clip.x, rc->clip.y, rc->clip.width, rc->clip.height);
                break;
            case RENDER_COMMAND_DISABLE_CLIP_RECT:
                glDisable(GL_SCISSOR_TEST);
                break;
            case RENDER_COMMAND_SET_BLEND_MODE:
                glBlendFunc(rc->blend.func0, rc->blend.func1);
                glBlendEquation(rc->blend.equation);
                break;
            case RENDER_COMMAND_SET_PROJECTION:
            case RENDER_COMMAND_SET_VIEW:
            case RENDER_COMMAND_MATRIX_UNIFORM:
                glUniformMatrix4fv(rc->uniform.location, 1, GL_FALSE, rc->uniform.m[0]);
                break;
            case RENDER_COMMAND_DRAW_VERTEX:
                // fprintf(stdout, "draw_vertex: %d %d %d\n", rc->draw.mode, rc->draw.start, rc->draw.count);
                glDrawArrays(rc->draw.mode, rc->draw.start, rc->draw.count);
                break;
            case RENDER_COMMAND_DRAW_INDEX:
                glDrawElements(rc->draw.mode, rc->draw.count, GL_UNSIGNED_INT, NULL);
            break;
            case RENDER_COMMAND_DRAW_VERTEX_INSTANCED:
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glDrawArraysInstanced(rc->instanced.mode, rc->instanced.start, rc->instanced.count, rc->instanced.instance_count);
        #endif
            break;
            case RENDER_COMMAND_DRAW_INDEX_INSTANCED:
        #if !defined(OS_EMSCRIPTEN) && !defined(OS_ANDROID)
                glDrawElementsInstanced(rc->instanced.mode, rc->instanced.count, GL_UNSIGNED_INT, NULL, rc->instanced.instance_count);
        #endif
            break;
            default:
                break;
        }
        it++;
        if (it >= block->current) {
            if (block_index < list->block_index) {
                block_index++;
                lua_rawgeti(L, LUA_REGISTRYINDEX, list->l_render_blocks_table_ref); 
                lua_rawgeti(L, -1, block_index);
                block = (struct RenderCommandBlock*)lua_touserdata(L, -1);
                lua_pop(L, 2);
                it = 0;
            } else break;
        }
    }
    return 0;
}

int l_RenderList_create(lua_State* L) {
    INIT_ARG();
    struct RenderList* list = (struct RenderList*)lua_newuserdata(L, sizeof(*list));
    luaL_setmetatable(L, "RenderList");
    // Set render list functions
    list->clear = l_default_RenderList_clear;
    list->push = l_default_RenderList_push;
    list->pop = l_default_RenderList_pop;
    list->call = l_default_RenderList_call;

    list->block_index = 1;
    lua_newtable(L);
    struct RenderCommandBlock* blk = (struct RenderCommandBlock*)lua_newuserdata(L, sizeof(*blk));
    list->block_ptr = blk;
    blk->current = 0;
    lua_rawseti(L, -2, 1);
    list->l_render_blocks_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 1;
}

int l_RenderList__destroy(lua_State* L) {
    CHECK_META(RenderList);
    luaL_unref(L, LUA_REGISTRYINDEX, self->l_render_blocks_table_ref);
    return 0;
}

int l_RenderList__push(lua_State* L) {
    return 0;
}

int l_RenderList__push_clear(lua_State* L) {
    CHECK_META(RenderList);
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
    RENDERLIST_PUSH(self, &rc);
    return 0;
}

int l_RenderList__pop(lua_State* L) {
    CHECK_META(RenderList);
    RENDERLIST_POP(self);
    return 1;
}

int l_RenderList__call(lua_State* L) {
    CHECK_META(RenderList);
    lua_pushcfunction(L, self->call);
    lua_pushvalue(L, 1);
    lua_call(L, 1, 0);
    return 0;
}

int l_RenderList_meta(lua_State* L) {
    luaL_newmetatable(L, "RenderList");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg reg[] = {
        REG_FIELD(RenderList, create),
        REG_META_FIELD(RenderList, push),
        REG_META_FIELD(RenderList, pop),
        REG_META_FIELD(RenderList, call),
        {"__gc", l_RenderList__destroy},
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
