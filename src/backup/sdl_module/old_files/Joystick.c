#include "selene_sdl.h"

// Meta

static META_FUNCTION(sdlJoystick, get_name) {
    CHECK_META(sdlJoystick);
    PUSH_STRING(SDL_JoystickName(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, get_vendor) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetVendor(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, get_product) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProduct(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, get_product_version) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, get_type) {
    CHECK_META(sdlJoystick);
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

static META_FUNCTION(sdlJoystick, get_type_string) {
    CHECK_META(sdlJoystick);
    PUSH_STRING(joy_types[SDL_JoystickGetType(*self)]);
    return 1;
}

static META_FUNCTION(sdlJoystick, num_axes) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumAxes(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, num_balls) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumBalls(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, num_hats) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumHats(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, num_buttons) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumButtons(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, get_axis) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*self, axis));
    return 1;
}

static META_FUNCTION(sdlJoystick, get_ball) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*self, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
    return 2;
}

static META_FUNCTION(sdlJoystick, get_hat) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*self, hat));
    return 1;
}

static META_FUNCTION(sdlJoystick, get_button) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_JoystickGetButton(*self, button));
    return 1;
}

static META_FUNCTION(sdlJoystick, rumble) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*self, low, high, freq) == 0);
    return 1;
}

static META_FUNCTION(sdlJoystick, current_power_level) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*self));
    return 1;
}

static META_FUNCTION(sdlJoystick, close) {
    CHECK_META(sdlJoystick);
    if (SDL_JoystickGetAttached(*self))
        SDL_JoystickClose(*self);
    return 0;
}

BEGIN_META(sdlJoystick) {
    BEGIN_REG(reg)
        REG_META_FIELD(sdlJoystick, get_name),
        REG_META_FIELD(sdlJoystick, get_vendor),
        REG_META_FIELD(sdlJoystick, get_product),
        REG_META_FIELD(sdlJoystick, get_product_version),
        REG_META_FIELD(sdlJoystick, get_type),
        REG_META_FIELD(sdlJoystick, get_type_string),
        REG_META_FIELD(sdlJoystick, num_axes),
        REG_META_FIELD(sdlJoystick, num_balls),
        REG_META_FIELD(sdlJoystick, num_hats),
        REG_META_FIELD(sdlJoystick, num_buttons),
        REG_META_FIELD(sdlJoystick, get_axis),
        REG_META_FIELD(sdlJoystick, get_ball),
        REG_META_FIELD(sdlJoystick, get_hat),
        REG_META_FIELD(sdlJoystick, get_button),
        REG_META_FIELD(sdlJoystick, rumble),
        REG_META_FIELD(sdlJoystick, current_power_level),
        REG_META_FIELD(sdlJoystick, close),
    END_REG()
    luaL_newmetatable(L, "sdlJoystick");
    luaL_setfuncs(L, reg, 0);

    // __index = self
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
