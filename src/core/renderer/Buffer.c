#include "selene_renderer.h"

MODULE_FUNCTION(Buffer, create) {
    INIT_ARG();
    int size = luaL_optinteger(L, 1, 1024);
    return 1;
}

int l_Buffer_open_meta(lua_State* L) {
    const luaL_Reg reg[] = {
        {NULL, NULL}
    };
    luaL_newmetatable(L, "Buffer");
    return 1;
}
