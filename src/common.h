#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
#else
#include "glad/glad.h"
#endif

#include "linmath.h"

#ifndef M_PI
#define M_PI 3.14159
#endif
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(v, a, b) (MAX(a, MIN(v, b)))
#define DEG2RAD(deg) ((deg)*(M_PI/180.0))
#define RAD2DEG(rad) ((rad)*(180.0/M_PI))
#define STR(x) #x

#endif /* COMMON_H_ */