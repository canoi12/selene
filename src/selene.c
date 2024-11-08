#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"

#ifdef SELENE_PLUGINS_PRELOAD
    // #include "plugins.h"
    extern int luaopen_plugins(lua_State* L);
#endif

static int _step_reg;
static int _quit_reg;
int selene_running = 0;

// Type Modules
extern int luaopen_AudioDecoder(lua_State* L);
extern int luaopen_Data(lua_State* L);
extern int luaopen_ImageData(lua_State* L);
extern int luaopen_FontData(lua_State* L);
extern int luaopen_MeshData(lua_State* L);

// Library Modules
extern int luaopen_fs(lua_State* L);
extern int luaopen_gl(lua_State* L);
extern int luaopen_linmath(lua_State* L);
#ifndef SELENE_NO_SDL
extern int luaopen_sdl(lua_State* L);
#endif

#ifndef SELENE_NO_SDL
static int l_load_from_sdl_rwops(lua_State* L) {
    const char* module_name = luaL_checkstring(L, 1);
    size_t len = strlen(module_name);
    char path[256];
    strcpy(path, module_name);
    for (int i = 0; i < len; i++) {
        if (path[i] == '.')
            path[i] = '/';
    }
    strcat(path, ".lua");
#if defined(OS_ANDROID)
    __android_log_print(ANDROID_LOG_DEBUG, "selene", "[%s] load file: %s", module_name, path);
#endif
    SDL_RWops* rw = SDL_RWFromFile(path, "r");
    if (!rw) {
        lua_pushfstring(L, "[selene] failed to open file: %s", path);
        return 1;
    }
    size_t size = SDL_RWsize(rw);
    char* content = malloc(size);
    SDL_RWread(rw, content, 1, size);
    if (luaL_loadbuffer(L, content, size, path) != LUA_OK) {
        free((void*)content);
        lua_pushfstring(L, "[selene] failed to parse Lua file: %s", path);
        return 1;
    }
    free((void*)content);
    return 1;
}
#endif

luaL_Reg _mod_regs[] = {
    {"fs", luaopen_fs},
    {"gl", luaopen_gl},
    {"linmath", luaopen_linmath},
#ifndef SELENE_NO_SDL
    {"sdl", luaopen_sdl},
#endif
    {NULL, NULL}
};

static int l_string_utf8codepoint(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(str);
    CHECK_INTEGER(pos);
    uint8_t* p = (uint8_t*)str + pos-1;
    if (*p < 0x80) {
        PUSH_INTEGER(*p);
        PUSH_INTEGER(1);
        return 2;
    }
    int codepoint = *p;
    int size = 1;

    switch (codepoint & 0xf0)
    {
        case 0xf0:
            codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
            size = 4;
            break;
        case 0xe0: {
            codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
            size = 3;
            break;
        }
        case 0xc0:
        case 0xd0: {
            codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
            size = 2;
            break;
        }
        default:
            codepoint = -1;
    }
    if (codepoint > SELENE_MAX_UNICODE) codepoint = -1;
    PUSH_INTEGER(codepoint);
    PUSH_INTEGER(size);
    return 2;
}

static int l_selene_get_exec_path(lua_State* L) {
    char path[1024];
#if defined(_WIN32)
    GetModuleFileNameA(NULL, path, 1024);
    int index = 0;
    char* p = path + strlen(path);
    while (*p != '\\') {
        p--;
    }
    p[1] = '\0';
    lua_pushstring(L, path);
#elif defined(__unix__) && !defined(__EMSCRIPTEN__)
    #if defined(__linux__)
    const char* proc = "/proc/self/exe";
    #elif defined(__FreeBSD__)
    const char* proc = "/proc/curproc/file";
    #else
    const char* proc = "/proc/self/path/a.out";
    #endif
    size_t len = readlink(proc, path, 1024);
    char* p = path + len;
    while (*p != '/') {
        p--;
    }
    p[1] = '\0';
    lua_pushstring(L, path);
#else
    lua_pushnil(L);
#endif
    return 1;
}

static int l_os_host(lua_State* L) {
#if defined(OS_WIN)
    lua_pushstring(L, "windows");
#elif defined(OS_LINUX)
    lua_pushstring(L, "linux");
#elif defined(OS_OSX)
    lua_pushstring(L, "macosx");
#elif defined(OS_EMSCRIPTEN)
    lua_pushstring(L, "emscripten");
#elif defined(OS_ANDROID)
    lua_pushstring(L, "android");
#elif defined(OS_BSD)
    lua_pushstring(L, "bsd");
#else
    lua_pushstring(L, "unknown");
#endif
    return 1;
}

static int l_os_arch(lua_State* L) {
#if defined(ARCH_x86)
    lua_pushstring(L, "x86");
#elif defined(ARCH_X64)
    lua_pushstring(L, "x64");
#elif defined(ARCH_ARM)
    lua_pushstring(L, "arm");
#else
    lua_pushnil(L);
#endif
return 1;
}

static void setup_extended_libs(lua_State* L) {
    /* String */
    lua_getglobal(L, "string");
    lua_pushcfunction(L, l_string_utf8codepoint);
    lua_setfield(L, -2, "utf8codepoint");
    lua_pop(L, 1);

    /* OS */
    lua_getglobal(L, "os");
    lua_pushcfunction(L, l_os_host);
    lua_setfield(L, -2, "host");
    lua_pushcfunction(L, l_os_arch);
    lua_setfield(L, -2, "arch");
    lua_pop(L, 1);
}

static int l_selene_get_version(lua_State* L) {
    PUSH_STRING(SELENE_VERSION);
    return 1;
}

static int l_selene_set_running(lua_State* L) {
    INIT_ARG();
    CHECK_BOOLEAN(running);
    selene_running = running;
    return 0;
}

#if !defined(SELENE_NO_STEP_FUNC)
static int l_selene_set_step(lua_State* L) {
    INIT_ARG();
    if (lua_type(L, arg) != LUA_TFUNCTION) return luaL_argerror(L, arg, "[selene] step must be a function");
    lua_pushvalue(L, arg);
    lua_rawsetp(L, LUA_REGISTRYINDEX, selene_run_step);
    return 0;
}
#endif
#if !defined(SELENE_NO_QUIT_FUNC)
static int l_selene_set_quit(lua_State* L) {
    INIT_ARG();
    if (lua_type(L, arg) != LUA_TFUNCTION) return luaL_argerror(L, arg, "[selene] step must be a function");
    lua_pushvalue(L, arg);
    lua_rawsetp(L, LUA_REGISTRYINDEX, selene_run_quit);
    return 0;
}
#endif

int luaopen_selene(lua_State* L) {
    luaL_Reg reg[] = {
        {"get_version", l_selene_get_version},
        {"set_running", l_selene_set_running},
        {"get_exec_path", l_selene_get_exec_path},
#if !defined(SELENE_NO_STEP_FUNC)
        {"set_step", l_selene_set_step},
#endif
#if !defined(SELENE_NO_QUIT_FUNC)
        {"set_quit", l_selene_set_quit},
#endif
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    LOAD_MODULE(AudioDecoder);
    LOAD_MODULE(Data);
    LOAD_MODULE(ImageData);
    LOAD_MODULE(FontData);
    LOAD_MODULE(MeshData);

#ifndef SELENE_NO_SDL
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchers");
    lua_pushcfunction(L, l_load_from_sdl_rwops);
    lua_rawseti(L, -2, lua_rawlen(L, -2)+1);
    lua_pop(L, 2);
#endif

    setup_extended_libs(L);
    int i;
    for (i = 0; _mod_regs[i].name != NULL; i++) {
        luaL_requiref(L, _mod_regs[i].name, _mod_regs[i].func, 1);
        lua_pop(L, 1);
    }

#ifdef SELENE_PLUGINS_PRELOAD
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, luaopen_plugins);
    lua_setfield(L, -2, "plugins");
    lua_pop(L, 2);
#endif

    return 1;
}

#if !defined(SELENE_NO_STEP_FUNC)
void selene_run_step(lua_State* L) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, selene_run_step);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
#if defined(OS_ANDROID)
        __android_log_print(ANDROID_LOG_ERROR, "selene", "failed to run step function: %s\n", lua_tostring(L, -1));
#else
        fprintf(stderr, "[selene] failed to run step function: %s\n", lua_tostring(L, -1));
#endif
        selene_running = 0;
    }
}
#endif
#if !defined(SELENE_NO_QUIT_FUNC)
void selene_run_quit(lua_State* L) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, selene_run_quit);
    if (lua_type(L, -1) == LUA_TFUNCTION) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
#if defined(OS_ANDROID)
            __android_log_print(ANDROID_LOG_ERROR, "selene", "failed to call quit callback: %s\n", lua_tostring(L, -1));
#else
            fprintf(stderr, "[selene] failed to call quit callback: %s\n", lua_tostring(L, -1));
#endif
    }
}
#endif

int selene_main(int argc, char** argv) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "selene", luaopen_selene, 1);
    // Setup args
    lua_newtable(L);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i+1);
    }
    lua_setfield(L, -2, "args");
    // Run init script
    if (luaL_dostring(L, selene_init_script) != LUA_OK) {
#if defined(OS_ANDROID)
         __android_log_print(ANDROID_LOG_ERROR, "selene", "failed to load main.lua: %s\n", lua_tostring(L, -1));
#else
        fprintf(stderr, "[selene] failed to load main.lua: %s\n", lua_tostring(L, -1));
#endif
        goto EXIT;
    }
    // Run main loop
#if defined(OS_EMSCRIPTEN)
    if (selene_running) emscripten_set_main_loop_arg((void(*)(void*))selene_run_step, L, 0, selene_running);
#else
    while (selene_running) selene_run_step(L);
#endif
    // Run quit callback
    selene_run_quit(L);
EXIT:
    fprintf(stdout, "[selene] exiting...\n");
    lua_close(L);
    return 0;
}
