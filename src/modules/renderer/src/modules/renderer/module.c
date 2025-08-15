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
        {"create", NULL},
        {NULL, NULL}
    };

    luaL_newmetatable(L, "selene.Renderer");
    const luaL_Reg _meta[] = {
        {"destroy", NULL},
        {NULL, NULL}
    };
    return 1;
}
