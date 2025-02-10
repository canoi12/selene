#include "selene.h"
#include "lua_helper.h"

#define SELENE_APP_CONTINUE 0
#define SELENE_APP_SUCCESS  1
#define SELENE_APP_FAILURE  2

static const char* s_boot_script =
"local status, err = pcall(function() require('main') end)\n"
"if not status then\n"
"   selene.set_running(false)\n"
"   error(err)\n"
"else\n"
"   selene.set_running(true)\n"
"end";

int selene_init(void** userdata, int argc, char** argv) {
    lua_State* L = (lua_State*)*userdata;
    luaL_openlibs(L);
#if SELENE_USE_JIT
    luaopen_ffi(L);
#endif
    luaL_requiref(L, "selene", luaopen_selene, 1);
    lua_newtable(L);
    for (int i = 1; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setfield(L, -2, "args");
#if !defined(OS_WIN) && !defined(OS_EMSCRIPTEN)
    const char* setup_path =
        "local path = selene.__exec\n"
        "if path and path ~= './' then\n"
        "   package.path = package.path .. ';' .. path .. '/?.lua;' .. path .. '?/init.lua'\n"
        "end";
    if (luaL_dostring(L, setup_path) != LUA_OK) {
        const char* msg = lua_tostring(L, -1);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[selene] failed to run pre-setup: %s\n", msg);
        return SELENE_APP_FAILURE;
    }
#endif

    const char* handle_args =
    "local args = selene.args\n"
    "for i=1,#args do\n"
    "   if args[i] == '-d' then\n"
    "       i = i + 1\n"
    "       selene.__dir = args[i]\n"
    "       package.path = selene.__dir .. '/?.lua;' .. selene.__dir .. '/?/init.lua;' .. package.path\n"
    "   elseif args[i] == '-f' then\n"
    "       i = i + 1\n"
    "       package.preload['boot'] = loadfile(args[i])\n"
    "   end\n"
    "end";

    if (luaL_dostring(L, handle_args) != LUA_OK) {
        const char* msg = lua_tostring(L, -1);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[selene] failed handle arguments: %s\n", msg);
        return SELENE_APP_FAILURE;
    }

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_getfield(L, -1, "boot");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_loadbuffer(L, s_boot_script, strlen(s_boot_script), "boot.lua");
        lua_setfield(L, -2, "boot");
    } else lua_pop(L, 1);
    lua_pop(L, 2);

    if (luaL_dostring(L, "require('boot')") != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[selene] Failed to load boot.lua: %s\n", error);
        return SELENE_APP_FAILURE;
    }
    return SELENE_APP_CONTINUE;
}

int selene_iterate(void* userdata) {
    lua_State* L = (lua_State*)userdata;
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.renderer_ref);
    lua_getfield(L, -1, "clear");
    lua_pushvalue(L, -2);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "[selene] failed to clear the renderer: %s",
                    lua_tostring(L, -1));
        return SELENE_APP_FAILURE;
    }
    lua_pop(L, 1);
    // SDL_Renderer** renderer = (SDL_Renderer**)lua_touserdata(L, -1);
    // lua_pop(L, 1);
    // SDL_SetRenderDrawColorFloat(*renderer, 0.3f, 0.4f, 0.4f, 1.0f);
    // SDL_RenderClear(*renderer);
    // glClearColor(0.3f, 0.4f, 0.4f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);

    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.step_callback_ref);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "[selene] failed to run step function: %s",
                    lua_tostring(L, -1));
        return SELENE_APP_FAILURE;
    }
    // SDL_RenderPresent(*renderer);
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.window_ref);
    if (lua_isuserdata(L, -1)) {
        SDL_Window** window = (SDL_Window**)lua_touserdata(L, -1);
        SDL_GL_SwapWindow(*window);
    }
    lua_pop(L, 1);
    
    return g_selene_context.is_running ? SELENE_APP_CONTINUE : SELENE_APP_SUCCESS;
}

#if defined(SELENE_USE_SDL3)
static int process_event(lua_State* L, SDL_Event* event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: {
            lua_pushstring(L, "quit");
            return 1;
        }
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN: {
            lua_pushstring(L, "key");
            lua_pushinteger(L, event->key.scancode);
            lua_pushboolean(L, event->key.down);
            lua_pushboolean(L, event->key.repeat);
            return 4;
        }
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            lua_pushstring(L, "window close");
            return 1;
        }
        case SDL_EVENT_WINDOW_MOVED: {
            lua_pushstring(L, "window moved");
            lua_pushinteger(L, event->window.data1);
            lua_pushinteger(L, event->window.data2);
            return 3;
        }
    }
    return 0;
}
#else
static int process_event(lua_State* L, SDL_Event* event) {
    switch (event->type) {
        case SDL_QUIT: {
            lua_pushstring(L, "quit");
            return 1;
        }
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            lua_pushstring(L, "key");
            lua_pushinteger(L, event->key.keysym.scancode);
            lua_pushboolean(L, event->type == SDL_KEYDOWN);
            lua_pushboolean(L, event->key.repeat);
            return 4;
        }
        case SDL_WINDOWEVENT: {
            switch (event->window.event) {
                case SDL_WINDOWEVENT_CLOSE: {
                    lua_pushstring(L, "window close");
                    return 1;
                }
                case SDL_WINDOWEVENT_MOVED: {
                    lua_pushstring(L, "window moved");
                    lua_pushinteger(L, event->window.data1);
                    lua_pushinteger(L, event->window.data2);
                    return 3;
                }
            }
        }
    }
    return 0;
}
#endif


int selene_event(void* userdata, SDL_Event* event) {
    lua_State* L = (lua_State*)userdata;
    // SDL_LockMutex(g_selene_context.event_mutex);
    // fprintf(stderr, "%d event: %d\n", g_selene_context.event_callback_ref, event->type);
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.event_callback_ref);
    if (lua_isfunction(L, -1)) {
        int res = process_event(L, event);
        if (lua_pcall(L, res, 0, 0) != LUA_OK) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "[selene] failed to run event function: %s",
                        lua_tostring(L, -1));
            return SELENE_APP_FAILURE;
        }
    } else lua_pop(L, 1);
    // SDL_UnlockMutex(g_selene_context.event_mutex);
    return SELENE_APP_CONTINUE;
}

void selene_quit(void* userdata, int status) {
    lua_State* L = (lua_State*)userdata;
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.quit_callback_ref);
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "[selene] failed to run quit function: %s",
                        lua_tostring(L, -1));
        }
    } else lua_pop(L, 1);
    if (g_selene_context.window_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.window_ref);
        if (lua_isuserdata(L, -1)) {
            SDL_Window** window = (SDL_Window**)lua_touserdata(L, -1);
            SDL_DestroyWindow(*window);
        }
        lua_pop(L, 1);
    }
    if (g_selene_context.initialized_sdl) SDL_Quit();
#if DEBUG
    #ifndef SELENE_NO_SDL
        SDL_Log("[selene] exiting...");
    #else
        fprintf(stdout, "[selene] exiting...\n");
    #endif
#endif
}

static void selene_run_step(void* userdata) {
    lua_State* L = (lua_State*)userdata;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        selene_event(L, &event);
    }
    selene_iterate(L);
}

int selene_main_loop(lua_State* L) {
    // Run main loop
#if defined(OS_EMSCRIPTEN)
    if (g_selene_context.is_running)
        emscripten_set_main_loop_arg((void (*)(void *))selene_run_step, L, 0,
                                     g_selene_context.is_running);
#else
    while (g_selene_context.is_running)
        selene_run_step(L);
#endif
    return 0;
}

int selene_main(int argc, char** argv) {
    lua_State* L = luaL_newstate();
    selene_init((void**)&L, argc, argv);
    selene_main_loop(L);
    selene_quit((void*)L, SELENE_APP_SUCCESS);
    lua_close(L);
    return 0;
}