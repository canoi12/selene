#include "lua_helper.h"
#include "sdl2.h"

static MODULE_FUNCTION(Event, create) {
  NEW_UDATA(Event, ev);
  return 1;
}

static META_FUNCTION(Event, poll) {
  CHECK_META(Event);
  PUSH_BOOLEAN(SDL_PollEvent(self));
  return 1;
}

static META_FUNCTION(Event, getType) {
  CHECK_META(Event);
  PUSH_INTEGER(self->type);
  return 1;
}

static META_FUNCTION(Event, windowEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->window.event);
  PUSH_INTEGER(self->window.windowID);
  PUSH_INTEGER(self->window.data1);
  PUSH_INTEGER(self->window.data2);
  return 4;
}

static META_FUNCTION(Event, mouseMotionEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->motion.which);
  PUSH_INTEGER(self->motion.x);
  PUSH_INTEGER(self->motion.y);
  PUSH_INTEGER(self->motion.xrel);
  PUSH_INTEGER(self->motion.yrel);
  PUSH_INTEGER(self->motion.state);
  return 6;
}

static META_FUNCTION(Event, mouseButtonEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->button.which);
  PUSH_INTEGER(self->button.button);
  PUSH_INTEGER(self->button.x);
  PUSH_INTEGER(self->button.y);
  PUSH_INTEGER(self->button.clicks);
  return 5;
}

static META_FUNCTION(Event, mouseWheelEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->wheel.which);
  PUSH_INTEGER(self->wheel.x);
  PUSH_INTEGER(self->wheel.y);
  PUSH_BOOLEAN(self->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
  return 4;
}

static META_FUNCTION(Event, keyboardEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->key.keysym.scancode);
  PUSH_INTEGER(self->key.state == SDL_PRESSED);
  PUSH_BOOLEAN(self->key.repeat);
  PUSH_INTEGER(self->key.keysym.sym);
  PUSH_INTEGER(self->key.keysym.mod);
  return 5;
}

static META_FUNCTION(Event, audioDeviceEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->adevice.which);
  return 1;
}

static META_FUNCTION(Event, gamepadDeviceEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->cdevice.which);
  return 1;
}

static META_FUNCTION(Event, gamepadAxisEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->caxis.which);
  PUSH_INTEGER(self->caxis.axis);
  PUSH_INTEGER(self->caxis.value);
  return 3;
}

static META_FUNCTION(Event, gamepadButtonEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->cbutton.which);
  PUSH_INTEGER(self->cbutton.button);
  PUSH_INTEGER(self->cbutton.state);
  return 3;
}

static META_FUNCTION(Event, joyDeviceEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->jdevice.which);
  return 1;
}

static META_FUNCTION(Event, joyAxisEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->jaxis.which);
  PUSH_INTEGER(self->jaxis.axis);
  PUSH_INTEGER(self->jaxis.value);
  return 1;
}

static META_FUNCTION(Event, joyBallEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->jball.which);
  PUSH_INTEGER(self->jball.ball);
  PUSH_INTEGER(self->jball.xrel);
  PUSH_INTEGER(self->jball.yrel);
  return 4;
}

static META_FUNCTION(Event, joyButtonEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->jbutton.which);
  PUSH_INTEGER(self->jbutton.button);
  PUSH_INTEGER(self->jbutton.state);
  return 3;
}

static META_FUNCTION(Event, joyHatEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->jhat.which);
  PUSH_INTEGER(self->jhat.hat);
  PUSH_INTEGER(self->jhat.value);
  return 3;
}

static META_FUNCTION(Event, dollarGestureEvent) {
  CHECK_META(Event);
  PUSH_INTEGER(self->dgesture.touchId);
  PUSH_INTEGER(self->dgesture.gestureId);
  PUSH_INTEGER(self->dgesture.numFingers);
  PUSH_NUMBER(self->dgesture.error);
  PUSH_NUMBER(self->dgesture.x);
  PUSH_NUMBER(self->dgesture.y);
  return 6;
}

static META_FUNCTION(Event, textEvent) {
  CHECK_META(Event);
  PUSH_STRING(self->text.text);
  return 1;
}

static META_FUNCTION(Event, textEditEvent) {
  CHECK_META(Event);
  PUSH_STRING(self->edit.text);
  PUSH_INTEGER(self->edit.start);
  PUSH_INTEGER(self->edit.length);
  return 3;
}

BEGIN_META(Event) {
  BEGIN_REG()
    REG_FIELD(Event, create),
  END_REG()

  BEGIN_REG(_index)
    REG_META_FIELD(Event, poll),
    REG_META_FIELD(Event, getType),
    REG_META_FIELD(Event, windowEvent),
    REG_META_FIELD(Event, audioDeviceEvent),
    REG_META_FIELD(Event, mouseMotionEvent),
    REG_META_FIELD(Event, mouseButtonEvent),
    REG_META_FIELD(Event, mouseWheelEvent),
    REG_META_FIELD(Event, keyboardEvent),
    REG_META_FIELD(Event, joyHatEvent),
    REG_META_FIELD(Event, textEvent),
    REG_META_FIELD(Event, textEditEvent),
    REG_META_FIELD(Event, gamepadDeviceEvent),
  END_REG()
  NEW_META(Event, _reg, _index_reg);
  return 1;
}
