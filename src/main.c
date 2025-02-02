#include "selene.h"
#include "lua_helper.h"

void selene_run_step(lua_State* L);
void selene_run_quit(lua_State* L);
int l_selene_set_running(lua_State *L);

static int main_symbol_test(int a) { return -1; }

static const char* s_boot_script =
    "local status, err = pcall(function() require('main') end)\n"
    "if not status then\n"
    "   runner.set_running(false)\n"
    "   error(debug.traceback(err, 1), 2)"
    "else\n"
    "   runner.set_running(true)\n"
    "   runner.run()\n"
    "end";

int selene_main(int argc, char** argv) {
    lua_State* L = luaL_newstate();
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
        goto EXIT;
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
        goto EXIT;
    }

    lua_pushstring(L, s_boot_script);
    lua_setglobal(L, "_BOOT_SCRIPT");

    const char* load_boot =
    "local status = pcall(function() require('boot') end)\n"
    "if not status then\n"
    "   package.preload['boot'] = load(_BOOT_SCRIPT, 'boot.lua')\n"
    "   require('boot')\n"
    "end";

    if (luaL_dostring(L, load_boot) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "selene: Failed to load boot.lua: %s\n", error);
    }
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

#ifndef OS_ANDROID
#if defined(MINGW)
    int SDL_main(int argc, char** argv) { return selene_main(argc, argv); }
#else
    int main(int argc, char** argv) { return selene_main(argc, argv); }
#endif
#endif