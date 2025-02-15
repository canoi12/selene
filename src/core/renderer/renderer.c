#include "renderer.h"
extern int l_renderer_Batch2D_open_meta(lua_State* L);

extern int l_Effect2D_open_meta(lua_State* L);
extern int l_Texture2D_open_meta(lua_State* L);

int l_Renderer_set_texture(lua_State* L) {
    CHECK_META(Renderer);
    return 0;
}

extern int l_renderer_create_Batch2D(lua_State* L);
extern int l_renderer_create_Render3D(lua_State* L);

int luaopen_renderer(lua_State* L) {
    lua_newtable(L);
    l_Effect2D_open_meta(L);
    lua_setfield(L, -2, EFFECT2D_CLASS);
    l_Texture2D_open_meta(L);
    lua_setfield(L, -2, TEXTURE2D_CLASS);
    l_renderer_Batch2D_open_meta(L);
    lua_setfield(L, -2, "Batch2D");
    const luaL_Reg reg[] = {
        {"create_batch2D", l_renderer_create_Batch2D},
        {NULL, NULL}
    };
    luaL_setfuncs(L, reg, 0);
    lua_newtable(L);
    l_selene_Batch2D_open_meta(L);
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "Batch2D");
    return 1;
}