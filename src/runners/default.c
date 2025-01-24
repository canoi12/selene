#include "selene.h"
#include "lua_helper.h"

#if !defined(SELENE_NO_STEP_FUNC)
static int l_selene_set_step(lua_State *L) {
  INIT_ARG();
  if (lua_type(L, arg) != LUA_TFUNCTION)
    return luaL_argerror(L, arg, "[selene] step must be a function");
  lua_pushvalue(L, arg);
  lua_rawsetp(L, LUA_REGISTRYINDEX, selene_run_step);
  return 0;
}
#endif
#if !defined(SELENE_NO_QUIT_FUNC)
static int l_selene_set_quit(lua_State *L) {
  INIT_ARG();
  if (lua_type(L, arg) != LUA_TFUNCTION)
    return luaL_argerror(L, arg, "[selene] step must be a function");
  lua_pushvalue(L, arg);
  lua_rawsetp(L, LUA_REGISTRYINDEX, selene_run_quit);
  return 0;
}
#endif


#if !defined(SELENE_NO_STEP_FUNC)
void selene_run_step(lua_State *L) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, selene_run_step);
  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
#ifndef SELENE_NO_SDL
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                  "failed to run step function: %s",
                  lua_tostring(L, -1));
#else
#if defined(OS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "selene",
                        "failed to run step function: %s\n",
                        lua_tostring(L, -1));
#else
    fprintf(stderr, "[selene] failed to run step function: %s\n",
            lua_tostring(L, -1));
#endif
#endif
    selene_running = 0;
  }
}
#endif
#if !defined(SELENE_NO_QUIT_FUNC)
void selene_run_quit(lua_State *L) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, selene_run_quit);
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
#ifndef SELENE_NO_SDL
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "[selene] failed to call quit callback: %s",
                    lua_tostring(L, -1));
#else
#if defined(OS_ANDROID)
      __android_log_print(ANDROID_LOG_ERROR, "selene",
                          "failed to call quit callback: %s\n",
                          lua_tostring(L, -1));
#else
      fprintf(stderr, "[selene] failed to call quit callback: %s\n",
              lua_tostring(L, -1));
#endif
#endif
    }
  }
}
#endif

luaL_Reg reg[] = {
#if !defined(SELENE_NO_STEP_FUNC)
    {"set_step", l_selene_set_step},
#endif
#if !defined(SELENE_NO_QUIT_FUNC)
    {"set_quit", l_selene_set_quit},
#endif
    {NULL, NULL}
};

int selene_main(int argc, char **argv) {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "selene", luaopen_selene, 1);
  lua_pushcfunction(L, reg[0].func);
  lua_setfield(L, -2, reg[0].name);
  lua_pushcfunction(L, reg[1].func);
  lua_setfield(L, -2, reg[1].name);
  // Setup args
  lua_newtable(L);
  for (int i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_setfield(L, -2, "args");
  // Run init script
  if (luaL_dostring(L, selene_init_script) != LUA_OK) {
#ifndef SELENE_NO_SDL
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "[selene] failed to load main.lua: %s",
                 lua_tostring(L, -1));
#else
#if defined(OS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "selene",
                        "failed to load main.lua: %s\n", lua_tostring(L, -1));
#else
    fprintf(stderr, "[selene] failed to load main.lua: %s\n",
            lua_tostring(L, -1));
#endif
#endif
    goto EXIT;
  }
  // Run main loop
#if defined(OS_EMSCRIPTEN)
  if (selene_running)
    emscripten_set_main_loop_arg((void (*)(void *))selene_run_step, L, 0,
                                 selene_running);
#else
  while (selene_running)
    selene_run_step(L);
#endif
  // Run quit callback
  selene_run_quit(L);
EXIT:
#if DEBUG
  #ifndef SELENE_NO_SDL
    SDL_Log("[selene] exiting...");
  #else
    fprintf(stdout, "[selene] exiting...\n");
  #endif
#endif
  lua_close(L);
  return 0;
}

