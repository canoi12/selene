#include "sdl2.h"

static MODULE_FUNCTION(Gamepad, open) {
    INIT_ARG();
    CHECK_INTEGER(index);
    SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (!ctrl) lua_pushnil(L);
    else {
        NEW_UDATA(Gamepad, controller);
        *controller = ctrl;
    }
    return 1;
}

static MODULE_FUNCTION(Gamepad, isValid) {
    INIT_ARG();
    CHECK_INTEGER(joy);
    PUSH_BOOLEAN(SDL_IsGameController(joy));
    return 1;
}

static META_FUNCTION(Gamepad, getName) {
    CHECK_META(Gamepad);
    PUSH_STRING(SDL_GameControllerName(*self));
    return 1;
}

static META_FUNCTION(Gamepad, getVendor) {
    CHECK_META(Gamepad);
    PUSH_INTEGER(SDL_GameControllerGetVendor(*self));
    return 1;
}

static META_FUNCTION(Gamepad, getProduct) {
    CHECK_META(Gamepad);
    PUSH_INTEGER(SDL_GameControllerGetProduct(*self));
    return 1;
}

static META_FUNCTION(Gamepad, getProductVersion) {
    CHECK_META(Gamepad);
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*self));
    return 1;
}

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static MODULE_FUNCTION(Gamepad, getAxisFromString) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
    return 1;
}

static MODULE_FUNCTION(Gamepad, getButtonFromString) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
    return 1;
}

static META_FUNCTION(Gamepad, getAxis) {
    CHECK_META(Gamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*self, axis));
    return 1;
}

static META_FUNCTION(Gamepad, getButton) {
    CHECK_META(Gamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetButton(*self, axis));
    return 1;
}

static META_FUNCTION(Gamepad, rumble) {
    CHECK_META(Gamepad);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    OPT_INTEGER(freq, 100);
    PUSH_NUMBER(SDL_GameControllerRumble(*self, low, high, freq));
    return 1;
}

const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};

static META_FUNCTION(Gamepad, currentPowerLevel) {
    CHECK_META(Gamepad);
    SDL_Joystick* j = SDL_GameControllerGetJoystick(*self);
    return 1;
}

static META_FUNCTION(Gamepad, close) {
    CHECK_META(Gamepad);
    if (SDL_GameControllerGetAttached(*self))
        SDL_GameControllerClose(*self);
    return 0;
}

BEGIN_META(Gamepad) {
    BEGIN_REG()
        REG_FIELD(Gamepad, open),
        REG_FIELD(Gamepad, isValid),
        REG_FIELD(Gamepad, getAxisFromString),
        REG_FIELD(Gamepad, getButtonFromString),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Gamepad, getName),
        REG_META_FIELD(Gamepad, getVendor),
        REG_META_FIELD(Gamepad, getProduct),
        REG_META_FIELD(Gamepad, getProductVersion),
        // REG_META_FIELD(Gamepad, getAxisFromString),
        REG_META_FIELD(Gamepad, getAxis),
        // REG_META_FIELD(Gamepad, getButtonFromString),
        REG_META_FIELD(Gamepad, getButton),
        REG_META_FIELD(Gamepad, rumble),
        REG_META_FIELD(Gamepad, currentPowerLevel),
        REG_META_FIELD(Gamepad, close),
    END_REG()
    NEW_META(Gamepad, _reg, _index_reg);
    return 1;
}
