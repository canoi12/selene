#include "selene.h"
#include "lua_helper.h"

static MODULE_FUNCTION(system, getOS) {
    #if defined(__EMSCRIPTEN__)
        PUSH_STRING("Emscripten");
    #elif defined(OS_WIN)
        PUSH_STRING("Windows");
    #elif defined(OS_OSX)
        PUSH_STRING("OSX");
    #elif defined(OS_LINUX)
        PUSH_STRING("Linux");
    #elif defined(OS_BSD)
        PUSH_STRING("BSD");
    #elif defined(OS_UNIX)
        PUSH_STRING("Unix");
    #endif
    return 1;
}

static MODULE_FUNCTION(system, getArch) {
    #if defined(ARCH_X86)
        PUSH_STRING("x86");
    #elif defined(ARCH_X64)
        PUSH_STRING("x64");
    #elif defined(ARCH_ARM)
        PUSH_STRING("arm");
    #endif
    return 1;
}

BEGIN_MODULE(system) {
    BEGIN_REG()
        REG_FIELD(system, getOS),
        REG_FIELD(system, getArch),
    END_REG()
    luaL_newlib(L, _reg);
    return 1;
}