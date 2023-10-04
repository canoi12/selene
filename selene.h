#ifndef SELENE_H_
#define SELENE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if defined(__WIN32) || defined(WIN32)
#define OS_WIN
#elif defined(__APPLE__)
#define OS_OSX
#elif defined(__linux__)
#define OS_LINUX
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#define OS_BSD
#elif defined(__unix__)
#define OS_UNIX
#endif

#if defined(__i386__) || defined(_M_IX86)
#define ARCH_X86
#elif defined(__amd64__) || defined(_M_X64)
#define ARCH_X64
#elif defined(__arm__) || defined(__ARM__) || defined(ARM) || defined(__ARM) || defined(__arm)
#define ARCH_ARM
#else
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#if !defined(__EMSCRIPTEN__)
#include "glad/glad.h"
#endif

#ifdef OS_WIN
// #define SDL_MAIN_HANDLED
    #include <SDL.h>
    #include <SDL_opengl.h>
#else
    #include <SDL2/SDL.h>
    #if !defined(__EMSCRIPTEN__)
        #include <SDL2/SDL_opengl.h>
    #else
        #include <SDL2/SDL_opengles2.h>
    #endif
#endif

#define SELENE_VER "0.1.0"

#ifndef M_PI
#define M_PI 3.14159
#endif
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(v, a, b) (MAX(a, MIN(v, b)))
#define DEG2RAD(deg) ((deg)*(M_PI/180.0))
#define RAD2DEG(rad) ((rad)*(180.0/M_PI))

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    int offset, size;
    void* data;
} Data;

int seleneopen_gl(lua_State* L);
int seleneopen_sdl2(lua_State* L);
int seleneopen_fs(lua_State* L);
int seleneopen_utils(lua_State* L);

int seleneopen_system(lua_State* L);

int luaopen_selene(lua_State* L);
#if defined(__cplusplus)
}
#endif

#endif // SELENE_H_
