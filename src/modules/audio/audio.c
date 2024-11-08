#include "audio.h"
#include "lua_helper.h"

int luaopen_audio(lua_State* L) {
    BEGIN_REG(reg)
    END_REG()
    luaL_newlib(L, reg);
    return 1;
}