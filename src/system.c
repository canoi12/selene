#include "selene.h"

static int l_system_os(lua_State* L) {
#if defined(__EMSCRIPTEN__)
    lua_pushstring(L, "Emscripten");
    return 1;
#endif
#if defined(OS_WIN)
    lua_pushstring(L, "Windows");
#elif defined(OS_OSX)
    lua_pushstring(L, "OSX");
#elif defined(OS_LINUX)
    lua_pushstring(L, "Linux");
#elif defined(OS_BSD)
    lua_pushstring(L, "BSD");
#elif defined(OS_UNIX)
    lua_pushstring(L, "Unix");
#endif
    return 1;
}

static int l_system_arch(lua_State* L) {
#if defined(ARCH_X86)
    lua_pushstring(L, "x86");
#elif defined(ARCH_X64)
    lua_pushstring(L, "x64");
#elif defined(ARCH_ARM)
    lua_pushstring(L, "arm");
#endif
    return 1;
}

int seleneopen_system(lua_State* L) {
    luaL_Reg reg[] = {
        {"os", l_system_os},
        {"arch", l_system_arch},
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    return 1;
}
