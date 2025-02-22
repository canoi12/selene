#include "renderer.h"

const char* pixel_formats[] = { "rgb", "rgba", NULL };
const int pixel_formats_values[] = { GL_RGB, GL_RGBA };

const char* texture_filters[] = { "nearest", "linear", NULL };
const int texture_filters_values[] = { GL_NEAREST, GL_LINEAR };

const char* draw_modes[] = {"lines", "triangles", NULL};
const int draw_modes_values[] = {GL_LINES, GL_TRIANGLES};

extern int l_Effect2D_open_meta(lua_State* L);
extern int l_Texture2D_open_meta(lua_State* L);

extern int l_Batch2D_open_meta(lua_State* L);

extern int l_Canvas_open_meta(lua_State* L);
extern int l_Canvas_create(lua_State* L);

extern int l_renderer_create_Batch2D(lua_State* L);
extern int l_renderer_create_Render3D(lua_State* L);

int luaopen_renderer(lua_State* L) {
    lua_newtable(L);
    l_Effect2D_open_meta(L);
    lua_setfield(L, -2, EFFECT2D_CLASS);
    l_Texture2D_open_meta(L);
    lua_setfield(L, -2, TEXTURE2D_CLASS);
    l_Batch2D_open_meta(L);
    lua_setfield(L, -2, "Batch2D");

    l_Canvas_open_meta(L);
    lua_setfield(L, -2, CANVAS_CLASS);
    const luaL_Reg reg[] = {
        {"create_batch2D", l_renderer_create_Batch2D},
        {"create_canvas", l_Canvas_create},
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    return 1;
}
