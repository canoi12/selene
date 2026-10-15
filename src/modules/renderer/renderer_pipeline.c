#include "modules/renderer.h"

int renderer_attrib_byte_stride(int type, int size) {
    switch (type) {
        case SELENE_BYTE:
        case SELENE_UBYTE:
            return size;
        case SELENE_SHORT:
        case SELENE_USHORT:
            return size * 2;
        case SELENE_DOUBLE:
            return size * 8;
        default:
            return size * 4;
    }
}

void renderer_apply_pipeline_desc(selene_RenderPipeline* pipe, const struct selene_PipelineDesc* desc) {
    if (!pipe || !desc) return;
    pipe->vs = desc->vs;
    pipe->ps = desc->ps;
    pipe->topology = desc->topology;
    pipe->layout = desc->layout;
    pipe->blend_state = desc->blend;
    pipe->depth_stencil_state = desc->depth;
    pipe->raster_state = desc->raster;
    pipe->binding_count = desc->binding_count;
    memcpy(pipe->bindings, desc->bindings, sizeof(desc->bindings));
}

int renderer_pipeline_binding_index(const selene_RenderPipeline* pipe, int binding_type, int slot) {
    if (!pipe) return slot;
    int fallback = -1;
    for (int i = 0; i < pipe->binding_count; i++) {
        if (pipe->bindings[i].type != binding_type) continue;
        if (pipe->bindings[i].slot == slot) return i;
        if (fallback < 0) fallback = i;
    }
    if (fallback >= 0) return fallback;
    /* Slot não encontrado no pipeline bindings — clamp ao binding count do layout. */
    if (slot >= pipe->binding_count) slot = pipe->binding_count > 0 ? pipe->binding_count - 1 : 0;
    return slot;
}

static int s_parse_stage_flags(lua_State* L, int idx, int fallback) {
    int flags = 0;
    int t = lua_type(L, idx);
    if (t == LUA_TSTRING) {
        const char* s = lua_tostring(L, idx);
        if (strcmp(s, "vertex") == 0) flags = SELENE_STAGE_VERTEX;
        else if (strcmp(s, "pixel") == 0 || strcmp(s, "fragment") == 0) flags = SELENE_STAGE_PIXEL;
        else if (strcmp(s, "compute") == 0) flags = SELENE_STAGE_COMPUTE;
    } else if (t == LUA_TTABLE) {
        int n = (int)lua_rawlen(L, idx);
        for (int i = 1; i <= n; i++) {
            lua_rawgeti(L, idx, i);
            flags |= s_parse_stage_flags(L, -1, 0);
            lua_pop(L, 1);
        }
    }
    return flags ? flags : fallback;
}

static int s_parse_binding_type(lua_State* L, int idx) {
    if (!lua_isstring(L, idx)) return SELENE_BINDING_TEXTURE;
    const char* s = lua_tostring(L, idx);
    if (strcmp(s, "uniform") == 0 || strcmp(s, "uniform_buffer") == 0)
        return SELENE_BINDING_UNIFORM;
    if (strcmp(s, "storage") == 0 || strcmp(s, "storage_buffer") == 0)
        return SELENE_BINDING_STORAGE;
    /* texture, combined_image_sampler, tex2d */
    return SELENE_BINDING_TEXTURE;
}

static void s_parse_bindings_table(lua_State* L, int tbl, struct selene_PipelineDesc* desc) {
    int n = (int)lua_rawlen(L, tbl);
    if (n > SELENE_PIPELINE_MAX_BINDINGS) n = SELENE_PIPELINE_MAX_BINDINGS;
    desc->binding_count = n;
    for (int i = 0; i < n; i++) {
        lua_rawgeti(L, tbl, i + 1);
        struct selene_PipelineBinding* b = &desc->bindings[i];
        memset(b, 0, sizeof(*b));
        b->slot = i;
        b->count = 1;
        b->type = SELENE_BINDING_TEXTURE;
        b->stage_flags = SELENE_STAGE_PIXEL;
        if (lua_istable(L, -1)) {
            if (lua_getfield(L, -1, "slot") == LUA_TNUMBER)
                b->slot = (int)lua_tointeger(L, -1);
            lua_pop(L, 1);

            if (lua_getfield(L, -1, "type") == LUA_TSTRING)
                b->type = s_parse_binding_type(L, -1);
            lua_pop(L, 1);

            if (lua_getfield(L, -1, "count") == LUA_TNUMBER) {
                int c = (int)lua_tointeger(L, -1);
                if (c > 0) b->count = c;
            }
            lua_pop(L, 1);

            int fallback = (b->type == SELENE_BINDING_UNIFORM)
                ? (SELENE_STAGE_VERTEX | SELENE_STAGE_PIXEL)
                : SELENE_STAGE_PIXEL;
            lua_getfield(L, -1, "stage");
            b->stage_flags = s_parse_stage_flags(L, -1, fallback);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
}

int renderer_parse_pipeline_desc(lua_State* L, int tbl, struct selene_PipelineDesc* desc) {
    if (!desc) return luaL_error(L, "invalid pipeline desc");
    memset(desc, 0, sizeof(*desc));
    if (!lua_istable(L, tbl))
        return luaL_argerror(L, tbl, "must be a table");

    lua_getfield(L, tbl, "vs");
    desc->vs = (selene_Shader*)luaL_checkudata(L, -1, selene_Shader_METANAME);
    lua_getfield(L, tbl, "ps");
    desc->ps = (selene_Shader*)luaL_checkudata(L, -1, selene_Shader_METANAME);
    lua_pop(L, 2);

    if (lua_getfield(L, tbl, "layout") != LUA_TTABLE)
        return luaL_error(L, "invalid layout field");
    if (lua_getfield(L, -1, "stride") == LUA_TNUMBER)
        desc->layout.vertex_stride = (size_t)lua_tointeger(L, -1);
    lua_pop(L, 1);

    int len = (int)lua_rawlen(L, -1);
    if (len > 8) len = 8;
    desc->layout.count = len;
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -1, i + 1);
        const char* name = "";
        if (lua_getfield(L, -1, "name") == LUA_TSTRING)
            name = lua_tostring(L, -1);
        strncpy(desc->layout.attributes[i].name, name, sizeof(desc->layout.attributes[i].name) - 1);
        desc->layout.attributes[i].name[sizeof(desc->layout.attributes[i].name) - 1] = '\0';
        lua_pop(L, 1);

        desc->layout.attributes[i].binding = i;
        if (lua_getfield(L, -1, "location") == LUA_TNUMBER)
            desc->layout.attributes[i].binding = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);

        desc->layout.attributes[i].offset = 0;
        if (lua_getfield(L, -1, "offset") == LUA_TNUMBER)
            desc->layout.attributes[i].offset = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);

        desc->layout.attributes[i].size = 0;
        if (lua_getfield(L, -1, "size") == LUA_TNUMBER)
            desc->layout.attributes[i].size = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "type");
        int opt = luaL_checkoption(L, -1, "float", type_name_options);
        lua_pop(L, 1);
        desc->layout.attributes[i].type = opt;
        desc->layout.attributes[i].stride = renderer_attrib_byte_stride(opt, desc->layout.attributes[i].size);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    desc->topology = SELENE_TOPOLOGY_TRIANGLES;
    if (lua_getfield(L, tbl, "topology") == LUA_TSTRING)
        desc->topology = luaL_checkoption(L, -1, "triangles", draw_modes);
    lua_pop(L, 1);

    desc->blend.enabled = 0;
    desc->blend.mode = SELENE_BLEND_ALPHA;
    if (lua_getfield(L, tbl, "blend") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN)
            desc->blend.enabled = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "func") == LUA_TSTRING)
            desc->blend.mode = luaL_checkoption(L, -1, "alpha", blend_mode_options);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    desc->depth.depth_enabled = 0;
    desc->depth.stencil_enabled = 0;
    desc->depth.depth_func = SELENE_COMPARE_LESS;
    desc->depth.depth_write_mask = 0;
    if (lua_getfield(L, tbl, "depth") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN)
            desc->depth.depth_enabled = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "func") == LUA_TSTRING)
            desc->depth.depth_func = luaL_checkoption(L, -1, "less", comparison_func_options);
        lua_pop(L, 1);
        desc->depth.depth_write_mask = desc->depth.depth_enabled;
        if (lua_getfield(L, -1, "write") == LUA_TBOOLEAN)
            desc->depth.depth_write_mask = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    desc->raster.fill_mode = SELENE_POLYGON_FILL;
    desc->raster.scissor_enabled = 0;
    desc->raster.cull_enabled = 0;
    desc->raster.front_face = SELENE_FRONT_CCW;
    desc->raster.cull_face = SELENE_CULL_BACK;
    if (lua_getfield(L, tbl, "cull") == LUA_TTABLE) {
        int has_enabled = 0;
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN) {
            desc->raster.cull_enabled = lua_toboolean(L, -1);
            has_enabled = 1;
        }
        lua_pop(L, 1);
        if (lua_getfield(L, -1, "front_face") == LUA_TSTRING)
            desc->raster.front_face = luaL_checkoption(L, -1, "ccw", front_face_options);
        lua_pop(L, 1);
        int has_face = 0;
        if (lua_getfield(L, -1, "cull_face") == LUA_TSTRING) {
            desc->raster.cull_face = luaL_checkoption(L, -1, "back", cull_face_options);
            has_face = 1;
        }
        lua_pop(L, 1);
        if (!has_enabled && has_face && desc->raster.cull_face != SELENE_CULL_NONE)
            desc->raster.cull_enabled = 1;
    }
    lua_pop(L, 1);

    if (lua_getfield(L, tbl, "scissor") == LUA_TTABLE) {
        if (lua_getfield(L, -1, "enabled") == LUA_TBOOLEAN)
            desc->raster.scissor_enabled = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    if (lua_getfield(L, tbl, "fill") == LUA_TSTRING)
        desc->raster.fill_mode = luaL_checkoption(L, -1, "fill", polygon_mode_options);
    lua_pop(L, 1);

    if (lua_getfield(L, tbl, "bindings") == LUA_TTABLE)
        s_parse_bindings_table(L, -1, desc);
    else {
        lua_pop(L, 1);
        if (lua_getfield(L, tbl, "descriptors") == LUA_TTABLE)
            s_parse_bindings_table(L, -1, desc);
    }
    lua_pop(L, 1);

    return 0;
}

void renderer_clip_correct(const selene_Renderer* r, mat4 proj) {
    if (!r || !proj) return;
    /* Selene clip: Y-up NDC like GL, Z [-1, 1]. Non-GL backends want Z [0, 1].
     * Vulkan framebuffer Y is flipped via negative viewport height. */
    if (r->backend == SELENE_RENDERER_OPENGL)
        return;

    mat4 fix;
    glm_mat4_identity(fix);
    fix[2][2] = 0.5f;
    fix[3][2] = 0.5f;
    mat4 tmp;
    glm_mat4_mul(fix, proj, tmp);
    glm_mat4_copy(tmp, proj);
}

void renderer_clip_ortho(const selene_Renderer* r,
                         float left, float right, float bottom, float top,
                         float zn, float zf, mat4 dest) {
    glm_mat4_identity(dest);
    glm_ortho(left, right, bottom, top, zn, zf, dest);
    renderer_clip_correct(r, dest);
}
