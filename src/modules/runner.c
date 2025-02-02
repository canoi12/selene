#include "selene.h"
#include "lua_helper.h"

#include "selene.h"
#include "lua_helper.h"

static int is_running = 0;
void runner_run_step(lua_State* L);
void runner_run_quit(lua_State* L);

void runner_run_step(lua_State* L) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, runner_run_step);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
#ifndef SELENE_NO_SDL
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "[selene] failed to run step function: %s",
                     lua_tostring(L, -1));
#else
    #if defined(OS_ANDROID)
        __android_log_print(ANDROID_LOG_ERROR, "selene",
                            "failed to run step function: %s\n",
                            lua_tostring(L, -1));
    #else
        fprintf(stderr,
                "[selene] failed to run step function: %s\n",
                lua_tostring(L, -1));
    #endif
#endif
        is_running = 0;
    }
}

void runner_run_quit(lua_State *L) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, runner_run_quit);
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

static int l_core_run(lua_State* L) {
    // Run main loop
#if defined(OS_EMSCRIPTEN)
    if (is_running)
        emscripten_set_main_loop_arg((void (*)(void *))runner_run_step, L, 0,
                                     is_running);
#else
    while (is_running)
        runner_run_step(L);
#endif
    runner_run_quit(L);
    return 0;
}

static int l_runner_set_step(lua_State *L) {
    INIT_ARG();
    if (lua_type(L, arg) != LUA_TFUNCTION)
        return luaL_argerror(L, arg, "[selene] step must be a function");
    lua_pushvalue(L, arg);
    lua_rawsetp(L, LUA_REGISTRYINDEX, runner_run_step);
    return 0;
}

static int l_runner_set_quit(lua_State *L) {
    INIT_ARG();
    if (lua_type(L, arg) != LUA_TFUNCTION)
        return luaL_argerror(L, arg, "[selene] step must be a function");
    lua_pushvalue(L, arg);
    lua_rawsetp(L, LUA_REGISTRYINDEX, runner_run_quit);
    return 0;
}

static int l_runner_set_running(lua_State* L) {
    INIT_ARG();
    CHECK_BOOLEAN(running);
    is_running = running;
    return 0;
}

int luaopen_runner(lua_State *L) {
    luaL_Reg reg[] = {
        {"run", l_core_run},
        {"set_step", l_runner_set_step},
        {"set_quit", l_runner_set_quit},
        {"set_running", l_runner_set_running},
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    return 1;
}