#include "selene.h"
#include "lua_helper.h"

#include "renderer/renderer.h"

static const SeleneContext* selctx = &g_selene_context;

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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[selene] failed to setup package.path: %s\n", msg);
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
    return g_selene_context.c_step_callback(L);
}

extern int g_selene_process_event(lua_State* L, SDL_Event* ev);

int selene_event(void* userdata, SDL_Event* event) {
    lua_State* L = (lua_State*)userdata;
    // SDL_LockMutex(selctx->event_mutex);
    // fprintf(stderr, "%d event: %d\n", selctx->event_callback_ref, event->type);
    int res = g_selene_process_event(L, event);
    if (lua_pcall(L, res, 0, 0) != LUA_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "[selene] failed to run event function: %s",
                    lua_tostring(L, -1));
        return SELENE_APP_FAILURE;
    }
    // SDL_UnlockMutex(selctx->event_mutex);
    return SELENE_APP_CONTINUE;
}

void selene_quit(void* userdata, int status) {
    lua_State* L = (lua_State*)userdata;
    g_selene_context.c_quit_callback(L, status);
}

static void selene_run_step(void* userdata) {
    lua_State* L = (lua_State*)userdata;
    SDL_Event event;
    lua_rawgeti(L, LUA_REGISTRYINDEX, selctx->l_event_callback_ref);
    if (lua_isfunction(L, -1)) {
#if 1
        while (SDL_PollEvent(&event)) {
            lua_pushvalue(L, -1);
            selene_event(L, &event);
        }
#endif
    }
    lua_pop(L, 1);
    selene_iterate(L);
}

int selene_main_loop(lua_State* L) {
    // Run main loop
#if defined(OS_EMSCRIPTEN)
    if (selctx->is_running)
        emscripten_set_main_loop_arg((void (*)(void *))selene_run_step, L, 0,
                                     selctx->is_running);
#else
    while (selctx->is_running)
        selene_run_step(L);
#endif
    return 0;
}

int selene_main(int argc, char** argv) {
    lua_State* L = luaL_newstate();
    selene_init((void**)&L, argc, argv);
    selene_main_loop(L);
    selene_quit((void*)L, SELENE_APP_SUCCESS);
    fprintf(stdout, "[selene] exiting...\n");
    lua_close(L);
    return 0;
}