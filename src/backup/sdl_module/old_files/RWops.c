#include "selene_sdl.h"

static MODULE_FUNCTION(sdlRWops, read) {
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(maxnum, 1);
    size_t read = SDL_RWread(self, data, size, maxnum);
    PUSH_INTEGER(read);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, write) {
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(num, 1);
    size_t written = SDL_RWwrite(self, data, size, num);
    PUSH_INTEGER(written);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, seek) {
    CHECK_META(sdlRWops);
    CHECK_INTEGER(offset);
    OPT_INTEGER(whence, RW_SEEK_SET);
    Sint64 res = SDL_RWseek(self, offset, whence);
    PUSH_INTEGER(res);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, size) {
    CHECK_META(sdlRWops);
    Sint64 size = SDL_RWsize(self);
    PUSH_INTEGER(size);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, tell) {
    CHECK_META(sdlRWops);
    Sint64 offset = SDL_RWtell(self);
    PUSH_INTEGER(offset);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, close) {
    CHECK_META(sdlRWops);
    Sint64 res = SDL_RWclose(self);
    PUSH_BOOLEAN(res == 0);
    return 1;
}

BEGIN_META(sdlRWops) {
    BEGIN_REG(reg)
        REG_FIELD(sdlRWops, read),
    END_REG()
    luaL_newmetatable(L, "sdlRWops");
    luaL_setfuncs(L, reg, 0);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}