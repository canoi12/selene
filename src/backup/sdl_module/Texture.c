#include "selene_sdl.h"

static META_FUNCTION(sdlTexture, destroy) {
    CHECK_META(sdlTexture);
    SDL_DestroyTexture(*self);
    return 0;
}

static META_FUNCTION(sdlTexture, query) {
    CHECK_META(sdlTexture);
    Uint32 format;
    int access;
    int w, h;
    SDL_QueryTexture(*self, &format, &access, &w, &h);
    PUSH_INTEGER(format);
    PUSH_INTEGER(access);
    PUSH_INTEGER(w);
    PUSH_INTEGER(h);
    return 4;
}

static META_FUNCTION(sdlTexture, set_alpha_mod) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(alpha);
    SDL_SetTextureAlphaMod(*self, (Uint8)alpha);
    return 0;
}

static META_FUNCTION(sdlTexture, set_color_mod) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(r);
    CHECK_INTEGER(g);
    CHECK_INTEGER(b);
    SDL_SetTextureColorMod(*self, r, g, b);
    return 0;
}

static META_FUNCTION(sdlTexture, set_blend_mode) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(blendMode);
    SDL_SetTextureBlendMode(*self, blendMode);
    return 0;
}

static META_FUNCTION(sdlTexture, set_scale_mode) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(scaleMode);
    SDL_SetTextureScaleMode(*self, scaleMode);
    return 0;
}

BEGIN_META(sdlTexture) {
    BEGIN_REG(reg)
        REG_META_FIELD(sdlTexture, destroy),
        REG_META_FIELD(sdlTexture, query),
        REG_META_FIELD(sdlTexture, set_alpha_mod),
        REG_META_FIELD(sdlTexture, set_color_mod),
        REG_META_FIELD(sdlTexture, set_blend_mode),
        REG_META_FIELD(sdlTexture, set_scale_mode),
    END_REG()
    luaL_newmetatable(L, "sdlTexture");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
