#include "selene.h"
#include "lua_helper.h"
#if defined(SELENE_USE_SDL3)

BEGIN_ENUM(sdl3_Enums)
    ENUM_FIELD(INIT_AUDIO, SDL_),
    ENUM_FIELD(INIT_VIDEO, SDL_),
    ENUM_FIELD(INIT_JOYSTICK, SDL_),
    ENUM_FIELD(INIT_GAMEPAD, SDL_),
    ENUM_FIELD(INIT_EVENTS, SDL_),
    ENUM_FIELD(INIT_CAMERA, SDL_),
    ENUM_FIELD(INIT_SENSOR, SDL_),
END_ENUM()


static int l_sdl3_init(lua_State* L) {
    int flags = luaL_checkinteger(L, 1);
    lua_pushboolean(L, SDL_Init(flags) == 0);
    return 1;
}

static int l_sdl3_quit(lua_State* L) {
    SDL_Quit();
    return 0;
}

static int l_sdl3_get_error(lua_State* L) {
    lua_pushstring(L, SDL_GetError());
    return 1;
}

static int l_sdl3_set_error(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    SDL_SetError("%s", msg);
    return 0;
}

static int l_sdl3_get_version(lua_State* L) {
    int version = SDL_GetVersion();
    lua_pushinteger(L, SDL_VERSIONNUM_MAJOR(version));
    lua_pushinteger(L, SDL_VERSIONNUM_MINOR(version));
    lua_pushinteger(L, SDL_VERSIONNUM_MICRO(version));
    return 3;
}

int luaopen_sdl(lua_State* L) {
    luaL_Reg reg[] = {
        {"init", l_sdl3_init},
        {"quit", l_sdl3_quit},
        {"get_error", l_sdl3_get_error},
        {"set_error", l_sdl3_set_error},
        {"get_version", l_sdl3_get_version},
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    luaL_newmetatable(L, "sdlWindow");
    lua_setfield(L, -2, "Window");
    return 1;
}
#endif
