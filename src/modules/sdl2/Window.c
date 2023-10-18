#include "sdl2.h"

static MODULE_FUNCTION(Window, Create) {
    INIT_ARG();
    CHECK_STRING(title);
    CHECK_INTEGER(x);
    CHECK_INTEGER(y);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int flags = 0;
    int top = lua_gettop(L);
    if (top >= arg) {
        if (lua_type(L, arg) != LUA_TTABLE)
            return luaL_argerror(L, arg, "Must be a table");
        PUSH_NIL();
        while (lua_next(L, arg) != 0) {
            flags |= (int)luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }

        NEW_UDATA(Window, window);
        *window = SDL_CreateWindow(
            title,
            x, y,
            width, height,
            flags
        );
    }
    return 1;
}

static META_FUNCTION(Window, GetSize) {
    INIT_ARG();
    CHECK_UDATA(Window, window);
    int width, height;
    SDL_GetWindowSize(*window, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
    return 2;
}

static META_FUNCTION(Window, SetSize) {
    return 0;
}

static META_FUNCTION(Window, GetPosition) {
    return 2;
}

static META_FUNCTION(Window, SetPosition) {
    return 0;
}

static META_FUNCTION(Window, Swap) {
    CHECK_META(Window);
    SDL_GL_SwapWindow(*self);
    return 0;
}

static META_FUNCTION(Window, SetBordered) {
    return 0;
}

static META_FUNCTION(Window, Maximize) {
    CHECK_META(Window);
    SDL_MaximizeWindow(*self);
    return 0;
}

static META_FUNCTION(Window, Minimize) {
    CHECK_META(Window);
    SDL_MinimizeWindow(*self);
    return 0;
}

static META_FUNCTION(Window, Restore) {
    CHECK_META(Window);
    SDL_RestoreWindow(*self);
    return 0;
}

static META_FUNCTION(Window, ShowSimpleMessageBox) {
    return 0;
}

static META_FUNCTION(Window, Destroy) {
    CHECK_META(Window);
    SDL_DestroyWindow(*self);
    return 0;
}

BEGIN_META(Window) {
    BEGIN_REG()
        REG_FIELD(Window, Create),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Window, GetSize),
        REG_META_FIELD(Window, SetSize),
        REG_META_FIELD(Window, GetPosition),
        REG_META_FIELD(Window, SetPosition),
        REG_META_FIELD(Window, Swap),
        REG_META_FIELD(Window, SetBordered),
        REG_META_FIELD(Window, Maximize),
        REG_META_FIELD(Window, Minimize),
        REG_META_FIELD(Window, Restore),
        REG_META_FIELD(Window, ShowSimpleMessageBox),
        REG_META_FIELD(Window, Destroy),
    END_REG()
    NEW_META(Window, _reg, _index_reg);
    return 1;
}
