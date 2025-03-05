#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main(int argc, char** argv) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dofile(L, "main.lua") != 0) {
        fprintf(stderr, "failed to run file main.lua: %s\n", lua_tostring(L, -1));
    }
    lua_close(L);
    return 0;
}
