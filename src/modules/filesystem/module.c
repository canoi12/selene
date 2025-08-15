#include "selene.h"

int r_exec_path = LUA_NOREF;
int r_dir_path = LUA_NOREF;
int r_user_path = LUA_NOREF;

static int l_filesystem_set_exec(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    lua_pushvalue(L, 1);
    if (r_exec_path == LUA_NOREF)
        r_exec_path = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawseti(L, LUA_REGISTRYINDEX, r_exec_path);
    return 0;
}

static int l_filesystem_get_exec(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, r_exec_path);
    return 1;
}

static int l_filesystem_set_dir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    lua_pushvalue(L, 1);
    if (r_dir_path == LUA_NOREF)
        r_dir_path = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawseti(L, LUA_REGISTRYINDEX, r_dir_path);
    return 0;
}

static int l_filesystem_get_dir(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, r_dir_path);
    return 1;
}

static int l_filesystem_set_user(lua_State* L) {
    const char* com = luaL_checkstring(L, 1);
    const char* app = luaL_checkstring(L, 2);
    const char* pref = SDL_GetPrefPath(com, app);
    lua_pushstring(L, pref);
    if (r_user_path == LUA_NOREF)
        r_user_path = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawseti(L, LUA_REGISTRYINDEX, r_user_path);
    return 0;
}

static int l_filesystem_get_user(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, r_exec_path);
    return 1;
}

int luaopen_filesystem(lua_State* L) {
    const luaL_Reg _reg[] = {
        {"set_exec", l_filesystem_set_exec},
        {"get_exec", l_filesystem_get_exec},
        {"set_dir", l_filesystem_set_dir},
        {"get_dir", l_filesystem_get_dir},
        {"set_user", l_filesystem_set_user},
        {"get_user", l_filesystem_get_user},
        {NULL, NULL}
    };
    return 1;
}
