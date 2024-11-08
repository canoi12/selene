#include "common.h"
#include "selene.h"

extern int luaopen_Data(lua_State* L);
extern int luaopen_FontData(lua_State* L);
extern int luaopen_ImageData(lua_State* L);

static const luaL_Reg _meta_regs[] = {
    {"Data", luaopen_Data},
    {"FontData", luaopen_FontData},
    {"ImageData", luaopen_ImageData},
    {NULL, NULL}
};

SELENE_API int luaopen_data_types(lua_State* L) {
    luaL_newlib(L, _meta_regs);
    return 1;
}