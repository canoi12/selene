#include "modules.h"

#define SELENE_VERSION "0.1.0"

extern MODULE_FUNCTION(Data, meta);

static int _running;
static int _core_reg;

static int l_selene_GetVersion(lua_State* L) {
    PUSH_STRING(SELENE_VERSION);
    return 1;
}

static int l_selene_IsRunning(lua_State* L) {
    PUSH_BOOLEAN(_running);
    return 1;
}

static int l_selene_SetRunning(lua_State* L) {
    INIT_ARG();
    CHECK_BOOLEAN(value);
    _running = value;
    return 0;
}

#define MAX_UNICODE 0x10FFFF
static MODULE_FUNCTION(selene, UTF8Codepoint) {
    INIT_ARG();
    CHECK_STRING(str);
    CHECK_INTEGER(pos);
    uint8_t* p = (uint8_t*)str + pos-1;
    if (*p < 0x80) {
        PUSH_INTEGER(*p);
        PUSH_INTEGER(1);
        return 2;
    }
    int codepoint = *p;
    int size = 1;

    switch (codepoint & 0xf0)
    {
        case 0xf0:
            codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
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
    if (codepoint > MAX_UNICODE) codepoint = -1;
    PUSH_INTEGER(codepoint);
    PUSH_INTEGER(size);
    return 2;
}

int luaopen_selene(lua_State* L) {
    BEGIN_REG()
        REG_FIELD(selene, GetVersion),
        REG_FIELD(selene, IsRunning),
        REG_FIELD(selene, SetRunning),
        REG_FIELD(selene, UTF8Codepoint),
    END_REG()
    luaL_newlib(L, _reg);

    LOAD_META(Data);

    LOAD_MODULE(audio);
    LOAD_MODULE(font);
    LOAD_MODULE(fs);
    LOAD_MODULE(gl);
    LOAD_MODULE(image);
    LOAD_MODULE(linmath);
    LOAD_MODULE(sdl2);
    LOAD_MODULE(stdc);
    LOAD_MODULE(system);

    return 1;
}

static void _step(void* _L) {
    lua_State* L = (lua_State*)_L;
    lua_rawgetp(L, LUA_REGISTRYINDEX, &_core_reg);
    lua_getfield(L, -1, "step");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* error_buf = lua_tostring(L, -1);
        fprintf(stderr, "Failed to call core step function: %s\n", error_buf);
        _running = 0;
        return;
    }
    lua_pop(L, 1);
}

const char* _boot =
"local sdl = selene.sdl2\n"
"local function add_path(path)\n"
"    package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path\n"
"end\n"
"return function()\n"
"    add_path(sdl.GetBasePath())\n"
"    local core = require 'core'\n"
"    core.init(selene.args)\n"
"    return core\n"
"end";

int main(int argc, char** argv) {
    lua_State* L = luaL_newstate();

    luaL_openlibs(L);
    luaL_requiref(L, "selene", luaopen_selene, 1);

    lua_createtable(L, 0, argc);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[1]);
        lua_rawseti(L, -2, i+1);
    }
    lua_setfield(L, -2, "args");

    if (luaL_dostring(L, _boot) != LUA_OK) {
        const char* error_buf = lua_tostring(L, -1);
        fprintf(stderr, "Failed to parse boot script: %s\n", error_buf);
        lua_close(L);
        return -1;
    }

    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        const char* error_buf = lua_tostring(L, -1);
        fprintf(stderr, "Failed to run boot function: %s\n", error_buf);
        lua_close(L);
        return -1;
    }
    lua_rawsetp(L, LUA_REGISTRYINDEX, &_core_reg);
    _running = 1;

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(_step, L, 0, 1);
#else
    while (_running) _step(L);
#endif

    lua_rawgetp(L, LUA_REGISTRYINDEX, &_core_reg);
    lua_getfield(L, -1, "quit");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* error_buf = lua_tostring(L, -1);
        fprintf(stderr, "Failed to call lua deinit function: %s\n", error_buf);
        lua_close(L);
        return -1;
    }
    lua_pop(L, 1);
    lua_close(L);
    return 0;
}
