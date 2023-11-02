#include "sdl2.h"

static MODULE_FUNCTION(Joystick, open) {
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

static MODULE_FUNCTION(Joystick, count) {
    PUSH_INTEGER(SDL_NumJoysticks());
    return 1;
}

// Meta

static META_FUNCTION(Joystick, getName) {
    CHECK_META(Joystick);
    PUSH_STRING(SDL_JoystickName(*self));
    return 1;
}

static META_FUNCTION(Joystick, getVendor) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickGetVendor(*self));
    return 1;
}

static META_FUNCTION(Joystick, getProduct) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickGetProduct(*self));
    return 1;
}

static META_FUNCTION(Joystick, getProductVersion) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*self));
    return 1;
}

static META_FUNCTION(Joystick, getType) {
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

static META_FUNCTION(Joystick, getTypeString) {
    CHECK_META(Joystick);
    PUSH_STRING(joy_types[SDL_JoystickGetType(*self)]);
    return 1;
}

static META_FUNCTION(Joystick, numAxes) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumAxes(*self));
    return 1;
}

static META_FUNCTION(Joystick, numBalls) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumBalls(*self));
    return 1;
}

static META_FUNCTION(Joystick, numHats) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumHats(*self));
    return 1;
}

static META_FUNCTION(Joystick, numButtons) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickNumButtons(*self));
    return 1;
}

static META_FUNCTION(Joystick, getAxis) {
    CHECK_META(Joystick);
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*self, axis));
    return 1;
}

static META_FUNCTION(Joystick, getBall) {
    CHECK_META(Joystick);
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*self, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
    return 2;
}

static META_FUNCTION(Joystick, getHat) {
    CHECK_META(Joystick);
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*self, hat));
    return 1;
}

static META_FUNCTION(Joystick, getButton) {
    CHECK_META(Joystick);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_JoystickGetButton(*self, button));
    return 1;
}

static META_FUNCTION(Joystick, rumble) {
    CHECK_META(Joystick);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*self, low, high, freq) == 0);
    return 1;
}

static META_FUNCTION(Joystick, currentPowerLevel) {
    CHECK_META(Joystick);
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*self));
    return 1;
}

static META_FUNCTION(Joystick, close) {
    CHECK_META(Joystick);
    if (SDL_JoystickGetAttached(*self))
        SDL_JoystickClose(*self);
    return 0;
}

BEGIN_META(Joystick) {
    BEGIN_REG()
        REG_FIELD(Joystick, open),
        REG_FIELD(Joystick, count),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(Joystick, getName),
        REG_META_FIELD(Joystick, getVendor),
        REG_META_FIELD(Joystick, getProduct),
        REG_META_FIELD(Joystick, getProductVersion),
        REG_META_FIELD(Joystick, getType),
        REG_META_FIELD(Joystick, getTypeString),
        REG_META_FIELD(Joystick, numAxes),
        REG_META_FIELD(Joystick, numBalls),
        REG_META_FIELD(Joystick, numHats),
        REG_META_FIELD(Joystick, numButtons),
        REG_META_FIELD(Joystick, getAxis),
        REG_META_FIELD(Joystick, getBall),
        REG_META_FIELD(Joystick, getHat),
        REG_META_FIELD(Joystick, getButton),
        REG_META_FIELD(Joystick, rumble),
        REG_META_FIELD(Joystick, currentPowerLevel),
        REG_META_FIELD(Joystick, close),
    END_REG()
    NEW_META(Joystick, _reg, _index_reg);
    return 1;
}
