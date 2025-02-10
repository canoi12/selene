#include "selene.h"
extern int l_selene_Batch2D_open_meta(lua_State* L);

int luaopen_renderer(lua_State* L) {
    lua_newtable(L);
    lua_newtable(L);
    l_selene_Batch2D_open_meta(L);
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "Batch2D");
    return 1;
}