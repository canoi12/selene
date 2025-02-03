#include "selene.h"
#include "lua_helper.h"

static int l_string_utf8codepoint(lua_State *L) {
    INIT_ARG();
    CHECK_STRING(str);
    CHECK_INTEGER(pos);
    uint8_t *p = (uint8_t *)str + pos - 1;
    if (*p < 0x80) {
        PUSH_INTEGER(*p);
        PUSH_INTEGER(1);
        return 2;
    }
    int codepoint = *p;
    int size = 1;

    switch (codepoint & 0xf0) {
    case 0xf0:
        codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) |
                    ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
        size = 4;
    break;
    case 0xe0: {
        codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
        size = 3;
    break;
    }
    case 0xc0:
    case 0xd0: {
        codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
        size = 2;
    break;
    }
    default:
        codepoint = -1;
    }
    if (codepoint > SELENE_MAX_UNICODE)
        codepoint = -1;
    PUSH_INTEGER(codepoint);
    PUSH_INTEGER(size);
    return 2;
}

static int l_os_host(lua_State *L) {
#if defined(OS_WIN)
    lua_pushstring(L, "windows");
#elif defined(OS_LINUX)
    lua_pushstring(L, "linux");
#elif defined(OS_OSX)
    lua_pushstring(L, "macosx");
#elif defined(OS_EMSCRIPTEN)
    lua_pushstring(L, "emscripten");
#elif defined(OS_ANDROID)
    lua_pushstring(L, "android");
#elif defined(OS_BSD)
    lua_pushstring(L, "bsd");
#elif defined(OS_NSWITCH)
    lua_pushstring(L, "switch");
#elif defined(OS_PS4)
    lua_pushstring(L, "ps4");
#elif defined(OS_PS5)
    lua_pushstring(L, "ps5");
#elif defined(OS_XBONE)
    lua_pushstring(L, "xbone");
#elif defined(OS_XB360)
    lua_pushstring(L, "xb360");
#else
    lua_pushstring(L, "unknown");
#endif
    return 1;
}

static int l_os_arch(lua_State *L) {
#if defined(ARCH_x86)
    lua_pushstring(L, "x86");
#elif defined(ARCH_X64)
    lua_pushstring(L, "x64");
#elif defined(ARCH_ARM)
    lua_pushstring(L, "arm");
#else
    lua_pushnil(L);
#endif
    return 1;
}

void l_setup_extended_libs(lua_State *L) {
      /* String */
      lua_getglobal(L, "string");
      lua_pushcfunction(L, l_string_utf8codepoint);
      lua_setfield(L, -2, "utf8codepoint");
      lua_pop(L, 1);

      /* OS */
      lua_getglobal(L, "os");
      lua_pushcfunction(L, l_os_host);
      lua_setfield(L, -2, "host");
      lua_pushcfunction(L, l_os_arch);
      lua_setfield(L, -2, "arch");
      lua_pop(L, 1);
}