#include "selene_sdl.h"

static META_FUNCTION(sdlGLContext, destroy) {
    CHECK_META(sdlGLContext);
    SDL_GL_DeleteContext(*self);
    return 0;
}

BEGIN_META(sdlGLContext) {
    BEGIN_REG(reg)
        REG_META_FIELD(sdlGLContext, destroy),
    END_REG()
    luaL_newmetatable(L, "sdlGLContext");
    luaL_setfuncs(L, reg, 0);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
