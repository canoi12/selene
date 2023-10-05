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

// Lua Module Helper

// Start a function for a new selene module
#define BEGIN_MODULE(name)\
int seleneopen_##name(lua_State* L) {

// End module
#define END_MODULE(ret)\
    return ret;\
}

#define NEW_MODULE(name)\
luaL_newlib(L, name##_reg)

#define MODULE_FUNCTION(name, func)\
int l_##name##_##func(lua_State* L)

#define BEGIN_FUNCTION(module, name)\
int l_##module##_##name(lua_State* L) {

#define END_FUNCTION(ret)\
    return ret;\
}

// Lua Enum Helper
#define BEGIN_ENUM(module)\
struct { const char* name; int value; } module##_Enums[] = {

#define END_ENUM()\
    {NULL, 0},\
};

#define ENUM_FIELD(name, ...)\
{#name, __VA_ARGS__##name}

#define LOAD_ENUMS(module)\
for (int i = 0; module##_Enums[i].name != NULL; i++) {\
    lua_pushinteger(L, module##_Enums[i].value);\
    lua_setfield(L, -2, module##_Enums[i].name);\
}

// Lua Meta Helper
#define NEW_META(type)\
luaL_newmetatable(L, #type);\
luaL_setfuncs(L, type##_reg, 0);\
lua_pushvalue(L, -1);\
lua_setfield(L, -2, "__index")

#define BEGIN_META(type)\
int l_##type##_meta(lua_State* L) {

#define END_META(ret)\
    return ret;}

#define BEGIN_REG(type)\
luaL_Reg type##_reg[] = {

#define END_REG()\
    {NULL, NULL}\
};

#define REG_FIELD(type, name)\
{#name, l_##type##_##name}

#define REG_META_FIELD(type, name)\
{#name, l_##type##__##name}

#define META_FUNCTION(type, func)\
int l_##type##__##func(lua_State* L)

#define BEGIN_META_FUNCTION(type, name, self)\
int l_##type##__##name(lua_State* L) {\
    INIT_GET_UDATA(type, self);

#define LOAD_META(type)\
l_##type##_meta(L);\
lua_setfield(L, -2, #type)

// Lua Helper
#define INIT_ARG()\
int arg = 1

#define NEW_UDATA(type, name, s)\
type* name = lua_newuserdata(L, s);\
luaL_setmetatable(L, #type)

#define GET_UDATA(type, name, ...)\
__VA_ARGS__##type* name = luaL_checkudata(L, arg++, #type)

#define INIT_GET_UDATA(type, name, ...)\
int arg = 1; __VA_ARGS__##type* name = luaL_checkudata(L, arg++, #type)

#define GET_NUMBER(type, name)\
type name = lua_tonumber(L, arg++)

#define GET_INTEGER(name)\
int name = (int)lua_tointeger(L, arg++)

#define GET_BOOLEAN(name)\
int name = lua_toboolean(L, arg++)

#define CHECK_NUMBER(type, name)\
type name = (type)luaL_checknumber(L, arg++)

#define CHECK_INTEGER(name)\
int name = (int)luaL_checkinteger(L, arg++)

#define CHECK_STRING(name)\
const char* name = luaL_checkstring(L, arg++)

#define OPT_NUMBER(type, name, def)\
type name = (type)luaL_optnumber(L, arg++, def)

#define OPT_INTEGER(name, def)\
int name = (int)luaL_optinteger(L, arg++, def)

#define PUSH_NUMBER(name)\
lua_pushnumber(L, name)

#define PUSH_INTEGER(name)\
lua_pushinteger(L, name)

#define PUSH_STRING(str)\
lua_pushstring(L, str)

#define PUSH_BOOLEAN(v)\
lua_pushboolean(L, v)

#define PUSH_NIL()\
lua_pushnil(L)

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
