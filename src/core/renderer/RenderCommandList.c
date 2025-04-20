#include "selene_renderer.h"

static void s_clear_render_list(RenderCommandList* list) {
    list->offset = 0;
}

static void s_push_render_command(RenderCommandList* list, int count, struct RenderCommand* cmd) {
    if (list->offset+count >= list->count) {
        list->count *= 2;
        list->commands = realloc(list->commands, sizeof(struct RenderCommand)*list->count);
    }
    struct RenderCommand* rc = list->commands + list->offset;
    memcpy(rc, cmd, count*sizeof(*cmd));
    list->offset += count;
}

static const struct RenderCommand* s_pop_render_command(RenderCommandList* list) {
    if (list->offset <= 0) return NULL;
    const struct RenderCommand* rc = list->commands + list->offset;
    list->offset--;
    return rc;
}

static void s_call_render_list(RenderCommandList* list) {
    for (int i = 0; i < list->offset; i++) {
        struct RenderCommand* rc = list->commands + i;
        switch (rc->type) {
            case RENDER_COMMAND_CLEAR: {
                float* c = rc->clear.color;
                glClearColor(c[0], c[1], c[2], c[3]);
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
                break;
            case RENDER_COMMAND_SET_FRAMEBUFFER:
                glBindFramebuffer(rc->target.target, rc->target.handle);
                break;
            case RENDER_COMMAND_SET_SCISSOR:
                glScissor(rc->clip.x, rc->clip.y, rc->clip.width, rc->clip.height);
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
            case RENDER_COMMAND_FLOAT_UNIFORM:
                glUniform1f(rc->uniform.location, rc->uniform.f);
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
            case RENDER_COMMAND_ENABLE_ATTRIB:
                glEnableVertexAttribArray(rc->attrib.index);
                break;
            case RENDER_COMMAND_DISABLE_ATTRIB:
                glDisableVertexAttribArray(rc->attrib.index);
                break;
            case RENDER_COMMAND_DISABLE_ALL_ATTRIBS: {
                int count;
                glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &count);
                for (int i = 0; i < count; i++) {
                    glDisableVertexAttribArray(i);
                }
            }
                break;
            case RENDER_COMMAND_VERTEX_ATTRIB_POINTER:
                glVertexAttribPointer(rc->attrib.index, rc->attrib.size, rc->attrib.type, rc->attrib.norm, rc->attrib.stride, (const void*)rc->attrib.offset);
                break;
            default:
                break;
        }
    }
}

int l_RenderCommandList_create(lua_State* L) {
    INIT_ARG();
    size_t count = luaL_optinteger(L, 1, 1024);
    struct RenderCommandList* list = (struct RenderCommandList*)lua_newuserdata(L, sizeof(*list));
    luaL_setmetatable(L, "RenderCommandList");
    // Set render list functions
    list->clear = s_clear_render_list;
    list->push = s_push_render_command;
    list->pop = s_pop_render_command;
    list->call = s_call_render_list;

    list->offset = 0;
    list->count = count;
    list->commands = (struct RenderCommand*)malloc(sizeof(struct RenderCommand)*count);
    return 1;
}

int l_RenderCommandList__destroy(lua_State* L) {
    CHECK_META(RenderCommandList);
    free(self->commands);
    return 0;
}

int l_RenderCommandList__push(lua_State* L) {
    CHECK_META(RenderCommandList);
    return 0;
}

MODULE_FUNCTION(RenderCommandList, _pop) {
    CHECK_META(RenderCommandList);
    self->pop(self);
    return 1;
}

int l_RenderCommandList__call(lua_State* L) {
    CHECK_META(RenderCommandList);
    self->call(self);
    return 0;
}

int l_RenderCommandList_meta(lua_State* L) {
    luaL_newmetatable(L, "RenderCommandList");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg reg[] = {
        REG_FIELD(RenderCommandList, create),
        REG_META_FIELD(RenderCommandList, push),
        REG_META_FIELD(RenderCommandList, pop),
        REG_META_FIELD(RenderCommandList, call),
        {"__gc", l_RenderCommandList__destroy},
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
