#ifndef MODULES_H_
#define MODULES_H_

#include "common.h"
#include "lua_helper.h"

#if defined(__cplusplus)
extern "C" {
#endif

int seleneopen_audio(lua_State* L);
int seleneopen_font(lua_State* L);
int seleneopen_fs(lua_State* L);
int seleneopen_gl(lua_State* L);
int seleneopen_image(lua_State* L);
int seleneopen_linmath(lua_State* L);
int seleneopen_sdl2(lua_State* L);
int seleneopen_system(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* MODULES_H_ */