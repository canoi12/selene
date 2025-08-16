#include "modules/window.h"
#include "lua_helper.h"
#if 1
#if defined(SELENE_USE_GLFW)
static int l_create_GLFW_window(lua_State* L) {
    return 1;
}
#else
extern int l_create_SDL_window(lua_State* L);
#endif

int l_create_window(lua_State* L) {
#if defined(SELENE_USE_GLFW)
    return l_create_GLFW_window(L);
#else
    return l_create_SDL_window(L);
#endif
}

extern int l_selene_Window__destroy(lua_State* L);
extern int l_selene_Window__get_size(lua_State* L);
extern int l_selene_Window__set_size(lua_State* L);

int luaopen_window(lua_State* L) {
    luaL_newmetatable(L, selene_Window_METANAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    const luaL_Reg _reg[] = {
        REG_META_FIELD(selene_Window, destroy),
        REG_META_FIELD(selene_Window, get_size),
        REG_META_FIELD(selene_Window, set_size),
        {NULL, NULL}
    };
    luaL_setfuncs(L, _reg, 0);
    return 1;
}
#endif