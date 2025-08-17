#include "modules/renderer.h"

// #include "font8x8/font8x8_latin.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef SELENE_NO_OPENGL
extern int l_GL_Renderer_create(lua_State* L);
static int l_gl_renderer_functions_ref = LUA_NOREF;
#endif             
#ifndef SELENE_NO_VULKAN
extern int l_VK_Renderer_create(lua_State* L);
#endif
static int l_vk_renderer_functions_ref = LUA_NOREF;
#if defined(OS_WIN)
extern int l_DX11_Renderer_meta(lua_State* L);
extern int l_DX11_Renderer_create(lua_State* L);
static int l_dx11_renderer_functions_ref = LUA_NOREF;

extern int l_DX12_Renderer_meta(lua_State* L);
static int l_dx12_renderer_functions_ref = LUA_NOREF;
#endif
#if defined(OS_APPLE)
extern int l_Metal_Renderer_meta(lua_State* L);
static int l_metal_renderer_functions_ref = LUA_NOREF;
#endif

static const char* blend_modes[] = {"alpha", "additive", "subtractive", "multiply", NULL};

const char* projection_modes[] = {"orthographic", "frustum", "perspective", NULL};

const char* clear_masks[] = {"color", "depth", "stencil", NULL};
const int clear_masks_values[] = {GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT };

const char* enable_attribs[] = { "blend", "depth test", "scissor test", "cull face", NULL };
const int enable_attribs_values[] = { GL_BLEND, GL_DEPTH_TEST, GL_SCISSOR_TEST, GL_CULL_FACE };

const char* pixel_formats[] = { "rgb", "rgba", "depth", NULL};
#if !defined(USE_GLES2)
const int pixel_formats_values[] = { GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT32 };
#endif

const char* texture_filters[] = { "nearest", "linear", NULL };
const int texture_filters_values[] = { GL_NEAREST, GL_LINEAR };

const char* texture_targets[] = { "tex2d", "tex3d", NULL };
const int texture_targets_values[] = { GL_TEXTURE_2D, GL_TEXTURE_3D };

const char* texture_target_options[] = {"tex2d", "tex3d", NULL};
const char* front_face_options[] = {"cw", "ccw", NULL};
const char* cull_face_options[] = {"none", "front", "back", "both", NULL};

const char* draw_modes[] = {"points", "lines", "triangles", NULL};
const int draw_modes_values[] = {GL_POINTS, GL_LINES, GL_TRIANGLES};

const char* shader_type_options[] = {"vertex", "pixel", NULL};
const char* buffer_target_options[] = {"vertex", "index", "uniform", NULL};

const char* polygon_mode_options[] = {"fill", "line", NULL};

const char* type_name_options[] = {"byte", "ubyte", "short", "ushort", "int", "uint", "float", "double", NULL};

const char* comparison_func_options[] = {"never", "less", "equal", "less equal", "greater", "not equal", "greater equal", "always", NULL};

const char* vertex_format_options[] = {"r32_float", "r32g32_float", "r32g32b32_float", "r32g32b32a32_float", NULL};

const char* renderer_backend_options[] = { "opengl", "vulkan", "dx11", "dx12", NULL };

int l_renderer_create(lua_State* L) {
    const char* api_name = luaL_checkstring(L, 2);
#ifndef SELENE_NO_OPENGL
    if (strcmp(api_name, "opengl") == 0) {
        return l_GL_Renderer_create(L);
    }
#endif
#if defined(OS_WIN)
    else if (strcmp(api_name, "d3d11") == 0) {
        return l_DX11_Renderer_create(L);
    }
#endif
#ifndef SELENE_NO_VULKAN
    else if (strcmp(api_name, "vulkan") == 0) {
        return l_VK_Renderer_create(L);
    }
#endif
    return luaL_argerror(L, 2, "invalid Renderer backend");
}

extern int l_Renderer_meta(lua_State* L);

extern int l_VertexBatch_meta(lua_State* L);
extern int l_VertexBatch_create(lua_State* L);

int luaopen_renderer(lua_State* L) {
    lua_newtable(L);
    // Renderer meta
    l_Renderer_meta(L);
    lua_setfield(L, -2, "Renderer");

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
