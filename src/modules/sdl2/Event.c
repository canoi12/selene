#include "sdl2.h"

static MODULE_FUNCTION(Event, Create) {
    NEW_UDATA(Event, ev);
    return 1;
}

static META_FUNCTION(Event, Poll) {
    CHECK_META(Event);
    PUSH_BOOLEAN(SDL_PollEvent(self));
    return 1;
}

static META_FUNCTION(Event, GetType) {
    CHECK_META(Event);
    PUSH_INTEGER(self->type);
    return 1;
}

static META_FUNCTION(Event, WindowEvent) {
    CHECK_META(Event);
    PUSH_INTEGER(self->window.event);
    PUSH_INTEGER(self->window.windowID);
    PUSH_INTEGER(self->window.data1);
    PUSH_INTEGER(self->window.data2);
    return 4;
}

static META_FUNCTION(Event, MouseMotionEvent) {
    CHECK_META(Event);
    PUSH_INTEGER(self->motion.which);
    PUSH_INTEGER(self->motion.state);
    PUSH_INTEGER(self->motion.x);
    PUSH_INTEGER(self->motion.y);
    PUSH_INTEGER(self->motion.xrel);
    PUSH_INTEGER(self->motion.yrel);
    return 6;
}


static META_FUNCTION(Event, MouseButtonEvent) {
    CHECK_META(Event);
    PUSH_INTEGER(self->button.which);
    PUSH_INTEGER(self->button.button);
    PUSH_INTEGER(self->button.x);
    PUSH_INTEGER(self->button.y);
    PUSH_INTEGER(self->button.clicks);
    return 5;
}

static META_FUNCTION(Event, MouseWheelEvent) {
    CHECK_META(Event);
    PUSH_INTEGER(self->wheel.which);
    PUSH_INTEGER(self->wheel.x);
    PUSH_INTEGER(self->wheel.y);
    return 4;
}

static META_FUNCTION(Event, KeyboardEvent) {
    CHECK_META(Event);
    PUSH_STRING(SDL_GetScancodeName(self->key.keysym.scancode));
    PUSH_BOOLEAN(self->key.repeat);
return 2;
}

static META_FUNCTION(Event, AudioDeviceEvent) {
    CHECK_META(Event);
    PUSH_INTEGER(self->adevice.which);
return 1;
}

static META_FUNCTION(Event, TextEvent) {
    CHECK_META(Event);
    return 1;
}

BEGIN_META(Event) {
    BEGIN_REG()
        REG_FIELD(Event, Create),
    END_REG()

    BEGIN_REG(_index)
        REG_META_FIELD(Event, Poll),
        REG_META_FIELD(Event, GetType),
        REG_META_FIELD(Event, WindowEvent),
        REG_META_FIELD(Event, AudioDeviceEvent),
        REG_META_FIELD(Event, MouseMotionEvent),
        REG_META_FIELD(Event, MouseButtonEvent),
        REG_META_FIELD(Event, KeyboardEvent),
    END_REG()
    NEW_META(Event, _reg, _index_reg);
    return 1;
}
