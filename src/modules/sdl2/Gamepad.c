#include "sdl2.h"

static MODULE_FUNCTION(Gamepad, Open) {
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

static MODULE_FUNCTION(Gamepad, IsValid) {
    INIT_ARG();
    CHECK_INTEGER(joy);
    PUSH_BOOLEAN(SDL_IsGameController(joy));
    return 1;
}

static META_FUNCTION(Gamepad, GetName) {
    CHECK_META(Gamepad);
    PUSH_STRING(SDL_GameControllerName(*self));
    return 1;
}

static META_FUNCTION(Gamepad, GetVendor) {
    CHECK_META(Gamepad);
    PUSH_INTEGER(SDL_GameControllerGetVendor(*self));
    return 1;
}

static META_FUNCTION(Gamepad, GetProduct) {
    CHECK_META(Gamepad);
    PUSH_INTEGER(SDL_GameControllerGetProduct(*self));
    return 1;
}

static META_FUNCTION(Gamepad, GetProductVersion) {
    CHECK_META(Gamepad);
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*self));
    return 1;
}

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static MODULE_FUNCTION(Gamepad, GetAxisFromString) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
    return 1;
}

static MODULE_FUNCTION(Gamepad, GetButtonFromString) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
    return 1;
}

static META_FUNCTION(Gamepad, GetAxis) {
    CHECK_META(Gamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*self, axis));
    return 1;
}

static META_FUNCTION(Gamepad, GetButton) {
    CHECK_META(Gamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetButton(*self, axis));
    return 1;
}

static META_FUNCTION(Gamepad, Rumble) {
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

static META_FUNCTION(Gamepad, CurrentPowerLevel) {
    CHECK_META(Gamepad);
    SDL_Joystick* j = SDL_GameControllerGetJoystick(*self);
    return 1;
}

static META_FUNCTION(Gamepad, Close) {
    CHECK_META(Gamepad);
    if (SDL_GameControllerGetAttached(*self))
        SDL_GameControllerClose(*self);
    return 0;
}

BEGIN_META(Gamepad) {
    BEGIN_REG()
        REG_FIELD(Gamepad, Open),
        REG_FIELD(Gamepad, IsValid),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Gamepad, GetName),
        REG_META_FIELD(Gamepad, GetVendor),
        REG_META_FIELD(Gamepad, GetProduct),
        REG_META_FIELD(Gamepad, GetProductVersion),
        // REG_META_FIELD(Gamepad, GetAxisFromString),
        REG_META_FIELD(Gamepad, GetAxis),
        // REG_META_FIELD(Gamepad, GetButtonFromString),
        REG_META_FIELD(Gamepad, GetButton),
        REG_META_FIELD(Gamepad, Rumble),
        REG_META_FIELD(Gamepad, CurrentPowerLevel),
        REG_META_FIELD(Gamepad, Close),
    END_REG()
    NEW_META(Gamepad, _reg, _index_reg);
    return 1;
}
