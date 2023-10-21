#ifndef SDL_H_
#define SDL_H_

#include "selene.h"
#include "lua_helper.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef SDL_AudioDeviceID AudioDeviceID;
typedef SDL_AudioStream* AudioStream;
typedef SDL_Window* Window;
typedef SDL_GLContext GLContext;
typedef SDL_Event Event;
typedef SDL_Joystick* Joystick;
typedef SDL_GameController* Gamepad;

typedef struct {
    int top, count;
    AudioStream* data;
    int* availables;
} AudioStreamPool;

#if defined(__cplusplus)
}
#endif

#endif /* SDL_H_ */
