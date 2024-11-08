#ifndef SELENE_SDL2_H_
#define SELENE_SDL2_H_

#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

#ifndef SDL_WINDOW_CLASSNAME
#define SDL_WINDOW_CLASSNAME "sdl.Window"
#endif

typedef SDL_Window* sdlWindow;
typedef SDL_GLContext* sdlGLContext;
typedef SDL_Renderer* sdlRenderer;
typedef SDL_Event sdlEvent;
typedef SDL_Texture* sdlTexture;

typedef SDL_RWops sdlRWops;

typedef SDL_Joystick* sdlJoystick;
typedef SDL_GameController* sdlGamepad;

typedef SDL_AudioDeviceID sdlAudioDeviceID;
typedef SDL_AudioStream* sdlAudioStream;

typedef struct {
    int top, count;
    sdlAudioStream* data;
    int* availables;
} AudioStreamPool;

typedef struct {
	FontData* font_data;
	sdlTexture* texture;
} sdlFont;

#endif /* SELENE_SDL2_H_ */