#include "selene_sdl.h"

static META_FUNCTION(sdlWindow, destroy) {
    CHECK_META(sdlWindow);
    SDL_DestroyWindow(*self);
    return 0;
}

static META_FUNCTION(sdlWindow, get_size) {
    INIT_ARG();
    CHECK_UDATA(sdlWindow, window);
    int width, height;
    SDL_GetWindowSize(*window, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
    return 2;
}

static META_FUNCTION(sdlWindow, set_size) {
    return 0;
}

static META_FUNCTION(sdlWindow, get_position) {
    CHECK_META(sdlWindow);
    int x, y;
    SDL_GetWindowPosition(*self, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static META_FUNCTION(sdlWindow, set_position) {
    return 0;
}

static META_FUNCTION(sdlWindow, gl_swap) {
    CHECK_META(sdlWindow);
    SDL_GL_SwapWindow(*self);
    return 0;
}

static META_FUNCTION(sdlWindow, set_bordered) {
    return 0;
}

static META_FUNCTION(sdlWindow, maximize) {
    CHECK_META(sdlWindow);
    SDL_MaximizeWindow(*self);
    return 0;
}

static META_FUNCTION(sdlWindow, minimize) {
    CHECK_META(sdlWindow);
    SDL_MinimizeWindow(*self);
    return 0;
}

static META_FUNCTION(sdlWindow, restore) {
    CHECK_META(sdlWindow);
    SDL_RestoreWindow(*self);
    return 0;
}

static META_FUNCTION(sdlWindow, show_simple_message_box) {
    return 0;
}

int l_sdlWindow_meta(lua_State* L) {
    BEGIN_REG(reg)
        REG_META_FIELD(sdlWindow, destroy),
        REG_META_FIELD(sdlWindow, get_size),
        REG_META_FIELD(sdlWindow, set_size),
        REG_META_FIELD(sdlWindow, get_position),
        REG_META_FIELD(sdlWindow, set_position),
        REG_META_FIELD(sdlWindow, gl_swap),
        REG_META_FIELD(sdlWindow, set_bordered),
        REG_META_FIELD(sdlWindow, maximize),
        REG_META_FIELD(sdlWindow, minimize),
        REG_META_FIELD(sdlWindow, restore),
        REG_META_FIELD(sdlWindow, show_simple_message_box),
    END_REG()
    luaL_newmetatable(L, "sdlWindow");
    luaL_setfuncs(L, reg, 0);

    // __index = self
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    return 1;
}