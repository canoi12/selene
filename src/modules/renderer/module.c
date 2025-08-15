#include "modules/renderer.h"

static int l_renderer_create(lua_State* L) {
    NEW_META(selene_Renderer, ren);
    return 1;
}

/**
 * Meta functions
 **/

static int l_renderer__destroy(lua_State* L) {
    return 0;
}

int luaopen_renderer(lua_State* L) {
    const luaL_Reg _reg[] = {
        {"create", l_renderer_create},
        {NULL, NULL}
    };
    luaL_newlib(L, _reg);

    luaL_newmetatable(L, "selene.Renderer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg _meta[] = {
        {"destroy", l_renderer__destroy},
        {NULL, NULL}
    };
    luaL_setfuncs(L, _meta, 0);
    lua_setfield(L, -2, "Renderer");
    return 1;
}
