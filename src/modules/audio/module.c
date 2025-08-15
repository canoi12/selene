#include "selene.h"

static int l_audio_open_device(lua_State* L) {
    return 1;
}

int luaopen_audio(lua_State *L) {
    const luaL_Reg _reg[] = {
        {"open_device", NULL},
        { NULL, NULL },
    };
    return 1;
}
