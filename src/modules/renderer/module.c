#include "modules/renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef SELENE_NO_OPENGL
extern int l_GL_Renderer_create(lua_State* L);
#endif
#ifndef SELENE_NO_VULKAN
extern int l_VK_Renderer_create(lua_State* L);
#endif
#if defined(OS_WIN)
extern int l_DX11_Renderer_create(lua_State* L);
#endif

const char* pixel_formats[] = { "rgb", "rgba", "depth", "bgra", NULL };
const char* texture_filters[] = { "nearest", "linear", NULL };
const char* draw_modes[] = { "points", "lines", "triangles", NULL };
const char* shader_type_options[] = { "vertex", "pixel", "compute", NULL };
const char* buffer_target_options[] = { "vertex", "index", "uniform", "storage", NULL };
const char* texture_target_options[] = { "tex2d", "tex3d", "cube", NULL };
const char* front_face_options[] = { "cw", "ccw", NULL };
const char* cull_face_options[] = { "none", "front", "back", "both", NULL };
const char* polygon_mode_options[] = { "fill", "line", NULL };
const char* type_name_options[] = { "byte", "ubyte", "short", "ushort", "int", "uint", "float", "double", NULL };
const char* comparison_func_options[] = {
    "never", "less", "equal", "less equal", "greater", "not equal", "greater equal", "always", NULL
};
const char* vertex_format_options[] = {
    "r32_float", "r32g32_float", "r32g32b32_float", "r32g32b32a32_float", NULL
};
const char* renderer_backend_options[] = {
    "opengl", "vulkan", "d3d11", "d3d12", "metal", "agc", "gnm", NULL
};
const char* blend_mode_options[] = { "alpha", "additive", "subtract", "multiply", NULL };
const char* index_type_options[] = { "uint16", "uint32", NULL };
const char* pipeline_binding_options[] = { "uniform", "texture", "storage", NULL };

int l_renderer_create(lua_State* L) {
    const char* api_name = luaL_checkstring(L, 2);
#ifndef SELENE_NO_OPENGL
    if (strcmp(api_name, "opengl") == 0) {
        return l_GL_Renderer_create(L);
    }
#endif
#if defined(OS_WIN) && !defined(SELENE_NO_D3D11)
    if (strcmp(api_name, "d3d11") == 0) {
        return l_DX11_Renderer_create(L);
    }
#endif
#ifndef SELENE_NO_VULKAN
    if (strcmp(api_name, "vulkan") == 0) {
        return l_VK_Renderer_create(L);
    }
#endif
    if (strcmp(api_name, "d3d12") == 0 ||
        strcmp(api_name, "metal") == 0 ||
        strcmp(api_name, "agc") == 0 ||
        strcmp(api_name, "gnm") == 0) {
        return luaL_error(L, "Renderer backend '%s' is not implemented yet", api_name);
    }
    return luaL_argerror(L, 2, "invalid Renderer backend");
}

extern int l_Renderer_meta(lua_State* L);
extern int l_FrameData_meta(lua_State* L);
extern int l_VertexBatch_meta(lua_State* L);
extern int l_VertexBatch_create(lua_State* L);

int luaopen_renderer(lua_State* L) {
    lua_newtable(L);
    l_Renderer_meta(L);
    lua_setfield(L, -2, "Renderer");

    l_FrameData_meta(L);
    luaL_ref(L, LUA_REGISTRYINDEX);

    luaL_newmetatable(L, selene_Font_METANAME);
    lua_setfield(L, -2, "Font");

    luaL_newmetatable(L, selene_GpuBuffer_METANAME);
    lua_setfield(L, -2, "GpuBuffer");

    luaL_newmetatable(L, selene_RenderPipeline_METANAME);
    lua_setfield(L, -2, "RenderPipeline");

    luaL_newmetatable(L, selene_RenderTarget_METANAME);
    lua_setfield(L, -2, "RenderTarget");

    luaL_newmetatable(L, selene_Shader_METANAME);
    lua_setfield(L, -2, "Shader");

    luaL_newmetatable(L, selene_Texture2D_METANAME);
    lua_setfield(L, -2, "Texture2D");

    l_VertexBatch_meta(L);
    luaL_ref(L, LUA_REGISTRYINDEX);

    const luaL_Reg reg[] = {
            REG_FIELD(renderer, create),
            {"VertexBatch", l_VertexBatch_create},
            {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
