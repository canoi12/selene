#include "selene_sdl.h"

static META_FUNCTION(sdlGamepad, get_name) {
    CHECK_META(sdlGamepad);
    PUSH_STRING(SDL_GameControllerName(*self));
    return 1;
}

static META_FUNCTION(sdlGamepad, get_vendor) {
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetVendor(*self));
    return 1;
}

static META_FUNCTION(sdlGamepad, get_product) {
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProduct(*self));
    return 1;
}

static META_FUNCTION(sdlGamepad, get_product_version) {
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*self));
    return 1;
}

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static META_FUNCTION(sdlGamepad, get_axis) {
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*self, axis));
    return 1;
}

static META_FUNCTION(sdlGamepad, get_button) {
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GameControllerGetButton(*self, button) == 1);
    return 1;
}

static META_FUNCTION(sdlGamepad, rumble) {
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    OPT_INTEGER(freq, 100);
    PUSH_NUMBER(SDL_GameControllerRumble(*self, low, high, freq));
    return 1;
}

const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};

static META_FUNCTION(sdlGamepad, current_power_level) {
    CHECK_META(sdlGamepad);
    SDL_Joystick* j = SDL_GameControllerGetJoystick(*self);
    int level = SDL_JoystickCurrentPowerLevel(j)+1;
    PUSH_STRING(gpad_powerlevels[level]);
    return 1;
}

static META_FUNCTION(sdlGamepad, close) {
    CHECK_META(sdlGamepad);
    if (SDL_GameControllerGetAttached(*self))
        SDL_GameControllerClose(*self);
    return 0;
}

BEGIN_META(sdlGamepad) {
    BEGIN_REG(reg)
        REG_META_FIELD(sdlGamepad, get_name),
        REG_META_FIELD(sdlGamepad, get_vendor),
        REG_META_FIELD(sdlGamepad, get_product),
        REG_META_FIELD(sdlGamepad, get_product_version),
        // REG_META_FIELD(sdlGamepad, get_axis_from_string),
        REG_META_FIELD(sdlGamepad, get_axis),
        // REG_META_FIELD(sdlGamepad, get_button_from_string),
        REG_META_FIELD(sdlGamepad, get_button),
        REG_META_FIELD(sdlGamepad, rumble),
        REG_META_FIELD(sdlGamepad, current_power_level),
        REG_META_FIELD(sdlGamepad, close),
    END_REG()
    luaL_newmetatable(L, "sdlGamepad");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
