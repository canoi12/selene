#ifndef SDL_H_
#define SDL_H_

#include "selene.h"

#if defined(OS_WIN)
// #define SDL_MAIN_HANDLED
    #include <SDL.h>
    #include <SDL_opengl.h>
#elif defined(__EMSCRIPTEN__)
    #include <SDL.h>
    #include <SDL2/SDL_opengles2.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
#endif

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
