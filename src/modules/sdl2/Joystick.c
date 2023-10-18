#include "sdl2.h"

static MODULE_FUNCTION(Joystick, Open) {
    INIT_ARG();
    CHECK_INTEGER(id);
    SDL_Joystick* j = SDL_JoystickOpen(id);
    if (j) {
        NEW_UDATA(Joystick, joy);
        *joy = j;
    }
    else PUSH_NIL();
    return 1;
}

static MODULE_FUNCTION(Joystick, Count) {
    PUSH_INTEGER(SDL_NumJoysticks());
    return 1;
}

// Meta

static META_FUNCTION(Joystick, GetName) {
    CHECK_META(Joystick);
    PUSH_STRING(SDL_JoystickName(*self));
    return 1;
}

static META_FUNCTION(Joystick, GetVendor) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickGetVendor(*self));
    return 1;
}

static META_FUNCTION(Joystick, GetProduct) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickGetProduct(*self));
    return 1;
}

static META_FUNCTION(Joystick, GetProductVersion) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*self));
    return 1;
}

static META_FUNCTION(Joystick, GetType) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickGetType(*self));
    return 1;
}

const char *joy_types[] = {
    [SDL_JOYSTICK_TYPE_UNKNOWN] = "Unknown",
    [SDL_JOYSTICK_TYPE_GAMECONTROLLER] = "GameController",
    [SDL_JOYSTICK_TYPE_WHEEL] = "Wheel",
    [SDL_JOYSTICK_TYPE_ARCADE_STICK] = "Arcadestick",
    [SDL_JOYSTICK_TYPE_FLIGHT_STICK] = "FlightStick",
    [SDL_JOYSTICK_TYPE_DANCE_PAD] = "DancePad",
    [SDL_JOYSTICK_TYPE_GUITAR] = "Guitar",
    [SDL_JOYSTICK_TYPE_DRUM_KIT] = "DrumKit",
    [SDL_JOYSTICK_TYPE_ARCADE_PAD] = "ArcadePad",
    [SDL_JOYSTICK_TYPE_THROTTLE] = "Throttle",
};

static META_FUNCTION(Joystick, GetTypeString) {
    CHECK_META(Joystick);
    PUSH_STRING(joy_types[SDL_JoystickGetType(*self)]);
    return 1;
}

static META_FUNCTION(Joystick, NumAxes) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumAxes(*self));
    return 1;
}

static META_FUNCTION(Joystick, NumBalls) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumBalls(*self));
    return 1;
}

static META_FUNCTION(Joystick, NumHats) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumHats(*self));
    return 1;
}

static META_FUNCTION(Joystick, NumButtons) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumButtons(*self));
    return 1;
}

static META_FUNCTION(Joystick, GetAxis) {
    CHECK_META(Joystick);
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*self, axis));
    return 1;
}

static META_FUNCTION(Joystick, GetBall) {
    CHECK_META(Joystick);
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*self, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
    return 2;
}

static META_FUNCTION(Joystick, GetHat) {
    CHECK_META(Joystick);
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*self, hat));
    return 1;
}

static META_FUNCTION(Joystick, GetButton) {
    CHECK_META(Joystick);
    CHECK_INTEGER(button);
    PUSH_INTEGER(SDL_JoystickGetButton(*self, button));
    return 1;
}

static META_FUNCTION(Joystick, Rumble) {
    CHECK_META(Joystick);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*self, low, high, freq) == 0);
    return 1;
}

static META_FUNCTION(Joystick, CurrentPowerLevel) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*self));
    return 1;
}

static META_FUNCTION(Joystick, Close) {
    CHECK_META(Joystick);
    if (SDL_JoystickGetAttached(*self))
        SDL_JoystickClose(*self);
    return 0;
}

BEGIN_META(Joystick) {
    BEGIN_REG()
        REG_FIELD(Joystick, Open),
        REG_FIELD(Joystick, Count),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Joystick, GetName),
        REG_META_FIELD(Joystick, GetVendor),
        REG_META_FIELD(Joystick, GetProduct),
        REG_META_FIELD(Joystick, GetProductVersion),
        REG_META_FIELD(Joystick, GetType),
        REG_META_FIELD(Joystick, GetTypeString),
        REG_META_FIELD(Joystick, NumAxes),
        REG_META_FIELD(Joystick, NumBalls),
        REG_META_FIELD(Joystick, NumHats),
        REG_META_FIELD(Joystick, NumButtons),
        REG_META_FIELD(Joystick, GetAxis),
        REG_META_FIELD(Joystick, GetBall),
        REG_META_FIELD(Joystick, GetHat),
        REG_META_FIELD(Joystick, GetButton),
        REG_META_FIELD(Joystick, Rumble),
        REG_META_FIELD(Joystick, CurrentPowerLevel),
        REG_META_FIELD(Joystick, Close),
    END_REG()
    NEW_META(Joystick, _reg, _index_reg);
    return 1;
}
