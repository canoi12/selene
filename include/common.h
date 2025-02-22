#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#if defined(OS_UNIX)
#include <unistd.h>
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define SELENE_MAX_UNICODE 0x10FFFF

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#ifndef M_PI2
    #define M_PI2 6.28318530717958647692
#endif
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(v, a, b) (MAX(a, MIN(v, b)))
#define DEG2RAD(deg) ((deg)*(M_PI/180.0))
#define RAD2DEG(rad) ((rad)*(180.0/M_PI))
#define STR(x) #x

#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <cglm/vec4.h>
#include <cglm/mat4.h>
#include <cglm/cam.h>
#include <cglm/affine.h>

#ifndef SELENE_NO_GL
#if defined(OS_WIN) || defined(OS_LINUX) || defined(OS_APPLE)
#include <glad/glad.h>
#endif
#endif

#ifndef SELENE_NO_SDL
#if defined(OS_EMSCRIPTEN)
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengles2.h>
    #define USE_GLES2 1
#else
    #if defined(SELENE_USE_SDL3)
        #include <SDL3/SDL.h>
        #if !defined(OS_ANDROID)
            #include <SDL3/SDL_opengl.h>
        #else
            #include <SDL3/SDL_opengles2.h>
            #define USE_GLES2 1
        #endif
    #else
        #include <SDL.h>
        #if !defined(OS_ANDROID)
            #include <SDL_opengl.h>
        #else
            #include <SDL_opengles2.h>
            #define USE_GLES2 1
        #endif
    #endif
#endif
#endif

#endif /* COMMON_H_ */
