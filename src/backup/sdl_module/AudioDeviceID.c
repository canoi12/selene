#include "selene_sdl.h"

static META_FUNCTION(sdlAudioDeviceID, pause) {
    CHECK_META(sdlAudioDeviceID);
    GET_BOOLEAN(pause);
    SDL_PauseAudioDevice(*self, pause);
    return 0;
}

static META_FUNCTION(sdlAudioDeviceID, close) {
    CHECK_META(sdlAudioDeviceID);
    PUSH_VALUE(-1);
    lua_rawget(L, LUA_REGISTRYINDEX);
    AudioStreamPool* pool = lua_touserdata(L, -1);
    free(pool->data);
    free(pool->availables);
    free(pool);
    lua_pop(L, 1);
    SDL_CloseAudioDevice(*self);
    return 0;
}

BEGIN_META(sdlAudioDeviceID) {
    BEGIN_REG(reg)
        REG_META_FIELD(sdlAudioDeviceID, pause),
        REG_META_FIELD(sdlAudioDeviceID, close),
    END_REG()
    luaL_newmetatable(L, "sdlAudioDeviceID");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
