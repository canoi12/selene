#include "selene.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static int _running = 0;

static int l_core_reg;

static int l_selene_version(lua_State* L) {
   lua_pushstring(L, SELENE_VER);
   return 1;
}

static int l_selene_running(lua_State* L) {
   int top = lua_gettop(L);
   if (top > 0)
      _running = lua_toboolean(L, 1);
   lua_pushboolean(L, _running);
   return 1;
}

int luaopen_selene(lua_State* L) {
   luaL_Reg reg[] = {
      {"version", l_selene_version},
      {"running", l_selene_running},
      {NULL, NULL}
   };
   luaL_newlib(L, reg);

   struct { char* name; lua_CFunction fn; } libs[] = {
      {"sdl2", seleneopen_sdl2},
      {"gl", seleneopen_gl},
      {"fs", seleneopen_fs},
      {"utils", seleneopen_utils},
      {"system", seleneopen_system},
      {NULL, NULL}
   };
   int i;
   for (i = 0; libs[i].name; i++) {
      if (libs[i].fn) {
         libs[i].fn(L);
         lua_setfield(L, -2, libs[i].name);
      }
      // printf("Module: %s %p, loaded\n", libs[i].name, libs[i].fn);
   }
   return 1;
}

static int selene_init(lua_State* L, int argc, char** argv) {
    fprintf(stderr, "Hello Selene\n");

    int flags = SDL_INIT_SENSOR;
    flags |= SDL_INIT_AUDIO |
       SDL_INIT_VIDEO |
       SDL_INIT_JOYSTICK |
       SDL_INIT_GAMECONTROLLER |
       SDL_INIT_TIMER |
       SDL_INIT_EVENTS;

    if (SDL_Init(flags)) {
       fprintf(stderr, "Failed to init SDL2: %s\n", SDL_GetError());
       exit(EXIT_FAILURE);
    }

    luaL_openlibs(L);
    luaL_requiref(L, "selene", luaopen_selene, 1);

    lua_getglobal(L, "selene");
    lua_newtable(L);
    int i;
    for (int i = 0; i < argc; i++) {
       lua_pushstring(L, argv[i]);
       lua_rawseti(L, -2, i+1);
    }
    lua_setfield(L, -2, "args");
    lua_pop(L, 1);

    if (luaL_dofile(L, "core/init.lua") != LUA_OK) {
       const char* error_buf = lua_tostring(L, -1);
       fprintf(stderr, "Failed to load selene core init: %s\n", error_buf);
       exit(EXIT_FAILURE);
    }
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &l_core_reg);

    lua_getfield(L, -1, "init");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
      const char* error_buf = lua_tostring(L, -1);
      fprintf(stderr, "Failed to load selene init function: %s\n", error_buf);
      exit(EXIT_FAILURE);
    }
    lua_pop(L, 1);
    _running = 1;
    return 0;
}

static void _step(void* userdata) {

    lua_State* L = userdata;
    lua_rawgetp(L, LUA_REGISTRYINDEX, &l_core_reg);
    lua_getfield(L, -1, "loop");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* error_buf = lua_tostring(L, -1);
        fprintf(stderr, "Failed to call step function: %s", error_buf);
        exit(EXIT_FAILURE);
    }
    lua_pop(L, 1);
}

static int selene_loop(lua_State* L) {
#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(_step, L, 0, 1);
#else
    while (_running) _step(L);
#endif
    return 0;
}

static int selene_quit(lua_State* L) {
   lua_rawgetp(L, LUA_REGISTRYINDEX, &l_core_reg);
   lua_getfield(L, -1, "deinit");
   if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
      const char* error_buf = lua_tostring(L, -1);
      fprintf(stderr, "Failed to call lua deinit function: %s\n", error_buf);
      exit(EXIT_FAILURE);
   }
   lua_pop(L, 1);
   lua_close(L);
   SDL_Quit();
   return 0;
}

int main(int argc, char** argv) {
   lua_State* L = luaL_newstate();
   selene_init(L, argc, argv);
   selene_loop(L);
   selene_quit(L);
   return 0;
}
