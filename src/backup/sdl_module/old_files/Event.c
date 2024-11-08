#include "selene_sdl.h"

static META_FUNCTION(sdlEvent, poll) {
    CHECK_META(sdlEvent);
    PUSH_BOOLEAN(SDL_PollEvent(self));
    return 1;
}

static META_FUNCTION(sdlEvent, get_type) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->type);
    return 1;
}

static META_FUNCTION(sdlEvent, window_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->window.event);
  PUSH_INTEGER(self->window.windowID);
  PUSH_INTEGER(self->window.data1);
  PUSH_INTEGER(self->window.data2);
  return 4;
}

static META_FUNCTION(sdlEvent, mouse_motion_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->motion.which);
  PUSH_INTEGER(self->motion.x);
  PUSH_INTEGER(self->motion.y);
  PUSH_INTEGER(self->motion.xrel);
  PUSH_INTEGER(self->motion.yrel);
  PUSH_INTEGER(self->motion.state);
  return 6;
}

static META_FUNCTION(sdlEvent, mouse_button_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->button.which);
  PUSH_INTEGER(self->button.button);
  PUSH_INTEGER(self->button.x);
  PUSH_INTEGER(self->button.y);
  PUSH_INTEGER(self->button.clicks);
  return 5;
}

static META_FUNCTION(sdlEvent, mouse_wheel_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->wheel.which);
  PUSH_INTEGER(self->wheel.x);
  PUSH_INTEGER(self->wheel.y);
  PUSH_BOOLEAN(self->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
  return 4;
}

static META_FUNCTION(sdlEvent, keyboard_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->key.keysym.scancode);
  PUSH_INTEGER(self->key.state == SDL_PRESSED);
  PUSH_BOOLEAN(self->key.repeat);
  PUSH_INTEGER(self->key.keysym.sym);
  PUSH_INTEGER(self->key.keysym.mod);
  return 5;
}

static META_FUNCTION(sdlEvent, audio_device_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->adevice.which);
  return 1;
}

static META_FUNCTION(sdlEvent, gamepad_device_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->cdevice.which);
  return 1;
}

static META_FUNCTION(sdlEvent, gamepad_axis_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->caxis.which);
  PUSH_INTEGER(self->caxis.axis);
  PUSH_INTEGER(self->caxis.value);
  return 3;
}

static META_FUNCTION(sdlEvent, gamepad_button_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->cbutton.which);
  PUSH_INTEGER(self->cbutton.button);
  PUSH_INTEGER(self->cbutton.state);
  return 3;
}

static META_FUNCTION(sdlEvent, joy_device_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->jdevice.which);
  return 1;
}

static META_FUNCTION(sdlEvent, joy_axis_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->jaxis.which);
  PUSH_INTEGER(self->jaxis.axis);
  PUSH_INTEGER(self->jaxis.value);
  return 3;
}

static META_FUNCTION(sdlEvent, joy_ball_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->jball.which);
  PUSH_INTEGER(self->jball.ball);
  PUSH_INTEGER(self->jball.xrel);
  PUSH_INTEGER(self->jball.yrel);
  return 4;
}

static META_FUNCTION(sdlEvent, joy_button_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->jbutton.which);
  PUSH_INTEGER(self->jbutton.button);
  PUSH_INTEGER(self->jbutton.state);
  return 3;
}

static META_FUNCTION(sdlEvent, joy_hat_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->jhat.which);
  PUSH_INTEGER(self->jhat.hat);
  PUSH_INTEGER(self->jhat.value);
  return 3;
}

static META_FUNCTION(sdlEvent, dollar_gesture_event) {
  CHECK_META(sdlEvent);
  PUSH_INTEGER(self->dgesture.touchId);
  PUSH_INTEGER(self->dgesture.gestureId);
  PUSH_INTEGER(self->dgesture.numFingers);
  PUSH_NUMBER(self->dgesture.error);
  PUSH_NUMBER(self->dgesture.x);
  PUSH_NUMBER(self->dgesture.y);
  return 6;
}

static META_FUNCTION(sdlEvent, text_event) {
  CHECK_META(sdlEvent);
  PUSH_STRING(self->text.text);
  return 1;
}

static META_FUNCTION(sdlEvent, text_edit_event) {
  CHECK_META(sdlEvent);
  PUSH_STRING(self->edit.text);
  PUSH_INTEGER(self->edit.start);
  PUSH_INTEGER(self->edit.length);
  return 3;
}

BEGIN_META(sdlEvent) {
    BEGIN_REG(reg)
        REG_META_FIELD(sdlEvent, poll),
        REG_META_FIELD(sdlEvent, get_type),
        REG_META_FIELD(sdlEvent, window_event),
        REG_META_FIELD(sdlEvent, audio_device_event),
        REG_META_FIELD(sdlEvent, mouse_motion_event),
        REG_META_FIELD(sdlEvent, mouse_button_event),
        REG_META_FIELD(sdlEvent, mouse_wheel_event),
        REG_META_FIELD(sdlEvent, keyboard_event),
        REG_META_FIELD(sdlEvent, joy_hat_event),
        REG_META_FIELD(sdlEvent, text_event),
        REG_META_FIELD(sdlEvent, text_edit_event),
        REG_META_FIELD(sdlEvent, gamepad_device_event),
    END_REG()
    luaL_newmetatable(L, "sdlEvent");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}