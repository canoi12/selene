#include "selene.h"
#include "lua_helper.h"

#include "selene_renderer.h"

int g_sdl_modules = 0;

extern int l_renderer_create(lua_State* L);
extern int g_init_renderer(lua_State* L, Renderer* r, SDL_Window* win);

int s_default_event_callback(lua_State *L);
int s_default_step_callback(lua_State* L);
void s_default_quit_callback(lua_State* L, int status);

static const char* s_sdl_init_flags_options[] = {
    "timer", "audio", "video", "joystick", "haptic", "gamepad", "events", "sensor", "all", NULL
};

static const int s_sdl_init_flags_values[] = {
    SDL_INIT_TIMER, SDL_INIT_AUDIO, SDL_INIT_VIDEO, SDL_INIT_JOYSTICK,
    SDL_INIT_HAPTIC, SDL_INIT_GAMECONTROLLER, SDL_INIT_EVENTS, SDL_INIT_SENSOR,
    SDL_INIT_EVERYTHING
};

SeleneContext g_selene_context = {
    .is_running = 0,

    .l_step_callback_ref = LUA_NOREF,
    .l_quit_callback_ref = LUA_NOREF,
    .l_event_callback_ref = LUA_NOREF,
#if 0
    .l_audio_system_ref = LUA_NOREF,
    .l_window_ref = LUA_NOREF,
    .l_renderer_ref = LUA_NOREF,


    .c_event_callback = NULL,
    .c_step_callback = s_default_step_callback,
    .c_quit_callback = s_default_quit_callback
#endif
};
SeleneContext *s_ctx = &g_selene_context;

#if 0
static int s_selene_step_callback(lua_State *L) {
    const int res = s_default_step_callback(L);
#if 0
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_ctx->l_renderer_ref);
    Renderer *r = (Renderer *)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (r && r->present) r->present(r, L);
#endif
    // SDL_Delay(16);
    return res;
}

static void s_selene_quit_callback(lua_State *L, int status) {
    s_default_quit_callback(L, status);
    if (s_ctx->l_audio_system_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_ctx->l_audio_system_ref);
        lua_getfield(L, -1, "destroy");
        lua_pushvalue(L, -2);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "Failed to destroy selene audio system");
        }
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, s_ctx->l_audio_system_ref);
    }
    if (s_ctx->l_renderer_ref != LUA_NOREF) {
        #if 0
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_ctx->l_renderer_ref);
        lua_getfield(L, -1, "destroy");
        lua_pushvalue(L, -2);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                      "Failed to destroy selene renderer");
        }
        lua_pop(L, 1);
        #endif
        luaL_unref(L, LUA_REGISTRYINDEX, s_ctx->l_renderer_ref);
    }
    if (s_ctx->l_window_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, s_ctx->l_window_ref);
        SDL_Window **window = (SDL_Window **)lua_touserdata(L, -1);
        SDL_DestroyWindow(*window);
        lua_pop(L, 1);
        luaL_unref(L, LUA_REGISTRYINDEX, s_ctx->l_window_ref);
    }
    SDL_Quit();
#if DEBUG
    SDL_Log("[selene] exiting...");
#endif
}
#endif

static int l_selene__call(lua_State *L) {
    INIT_ARG();
    arg++;
#if 0
    const char *name = luaL_checkstring(L, 2);
    const char *version = luaL_checkstring(L, 3);
    const char *org = luaL_checkstring(L, 4);
#endif
    int flags = 0;
    if (!lua_istable(L, arg)) {
        flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER;
    }
    else {
        int len = lua_rawlen(L, 1);
        for (int i = 0; i < len; i++) {
            int opt = luaL_checkoption(L, arg, NULL, s_sdl_init_flags_options);
            flags |= s_sdl_init_flags_values[opt];
        }
    }
    
    if (SDL_Init(flags) < 0) {
        return luaL_error(L, "failed to initialize SDL: %s", SDL_GetError());
    }
#if 0
#if defined(OS_EMSCRIPTEN) || defined(OS_ANDROID)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    int width = 640;
    int height = 380;
    int flags = SDL_WINDOW_SHOWN;
    int backend = SELENE_RENDERER_OPENGL;

    int config_table = lua_gettop(L);
    if (lua_istable(L, config_table)) {
        // fprintf(stdout, "is table\n");
        lua_getfield(L, config_table, "window");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
        }
        lua_getfield(L, -1, "width");
        width = (int)luaL_optinteger(L, -1, width);
        lua_pop(L, 1);
        lua_getfield(L, -1, "height");
        height = (int)luaL_optinteger(L, -1, height);
        lua_pop(L, 1);
        lua_getfield(L, -1, "resizable");
        if (lua_isboolean(L, -1)) flags |= SDL_WINDOW_RESIZABLE * lua_toboolean(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "always_on_top");
        if (lua_isboolean(L, -1)) flags |= SDL_WINDOW_ALWAYS_ON_TOP * lua_toboolean(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "borderless");
        if (lua_isboolean(L, -1)) flags |= SDL_WINDOW_BORDERLESS * lua_toboolean(L, -1);
        lua_pop(L, 2);

        if (lua_getfield(L, config_table, "renderer") == LUA_TTABLE) {
            if (lua_getfield(L, -1, "backend") == LUA_TSTRING) backend = luaL_checkoption(L, -1, "opengl", renderer_backend_options);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    if (backend == SELENE_RENDERER_OPENGL) flags |= SDL_WINDOW_OPENGL;

    SDL_Window *win = SDL_CreateWindow(
        name,
#if !defined(SELENE_USE_SDL3)
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
#endif
        width, height, flags
    );
    if (win == NULL)
        return luaL_error(L, "failed to create window: %s", SDL_GetError());
    SDL_Window **win_ptr =
        (SDL_Window **)lua_newuserdata(L, sizeof(SDL_Window *));
    luaL_setmetatable(L, "sdlWindow");
    *win_ptr = win;
    g_selene_context.l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);
#if 0
    NEW_UDATA(Renderer, r);
    if (g_init_renderer(L, r, win) < 0) {
        const char* str = lua_tostring(L, -1);
        return luaL_error(L, "%s", str);
    }
#endif
    lua_pushcfunction(L, l_renderer_create);
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_window_ref);
    lua_pushstring(L, renderer_backend_options[backend]);

    if (lua_pcall(L, 2, 1, 0) != LUA_OK)
        return luaL_error(L, "failed to create renderer: %s", lua_tostring(L, -1));
    g_selene_context.l_renderer_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    g_selene_context.c_step_callback = s_selene_step_callback;
    g_selene_context.c_quit_callback = s_selene_quit_callback;
#endif
    return 0;
}

#if SELENE_USE_JIT
void luaL_requiref(lua_State *L, const char *modname, lua_CFunction openf, int glb) {
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname); /* argument to open function */
    lua_call(L, 1, 1);          /* open module */
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_pushvalue(L, -2);         /* make copy of module (call result) */
    lua_setfield(L, -2, modname); /* _LOADED[modname] = module */
    lua_pop(L, 1);                /* remove _LOADED table */
    if (glb) {
        lua_pushvalue(L, -1);      /* copy of 'mod' */
        lua_setglobal(L, modname); /* _G[modname] = module */
    }
}
#endif

extern void l_setup_extended_libs(lua_State *L);

// Global Modules
extern int luaopen_fs(lua_State *L);
extern int luaopen_gl(lua_State *L);
extern int luaopen_linmath(lua_State *L);
#ifndef SELENE_NO_JSON
extern int luaopen_json(lua_State *L);
#endif
#ifndef SELENE_NO_IMAGE
extern int luaopen_image(lua_State *L);
#endif
#ifndef SELENE_NO_FONT
extern int luaopen_font(lua_State *L);
#endif
#ifndef SELENE_NO_SDL
extern int luaopen_sdl(lua_State *L);
#endif
extern int luaopen_ctypes(lua_State *L);

static const luaL_Reg _global_modules_reg[] = {
    // {"fs", luaopen_fs},
#ifndef SELENE_NO_GL
    {"gl", luaopen_gl},
#endif
    {"linmath", luaopen_linmath},
#ifndef SELENE_NO_JSON
    {"json", luaopen_json},
#endif
#ifndef SELENE_NO_FONT
    {"font", luaopen_font},
#endif
#ifndef SELENE_NO_IMAGE
    {"image", luaopen_image},
#endif
#ifndef SELENE_NO_SDL
    {"sdl", luaopen_sdl},
#endif
    {"ctypes", luaopen_ctypes},
    {NULL, NULL}
};

// Selene Modules
extern int luaopen_audio(lua_State *L);
extern int luaopen_renderer(lua_State *L);
extern int luaopen_filesystem(lua_State* L);

static const luaL_Reg _selene_modules_reg[] = {
    {"audio", luaopen_audio},
    {"filesystem", luaopen_filesystem},
    {"renderer", luaopen_renderer},
    {NULL, NULL}
};

#ifndef SELENE_NO_SDL
static int l_load_from_sdl_rwops(lua_State *L) {
    const char *module_name = luaL_checkstring(L, 1);
    size_t len = strlen(module_name);
    char path[256];
    strcpy(path, module_name);
    for (int i = 0; i < len; i++) {
        if (path[i] == '.')
          path[i] = '/';
    }
    strcat(path, ".lua");
#if DEBUG && defined(OS_ANDROID)
    __android_log_print(ANDROID_LOG_DEBUG, "selene", "[%s] load file: %s",
                        module_name, path);
#endif
#if defined(SELENE_USE_SDL3)
    SDL_IOStream *rw = SDL_IOFromFile(path, "r");
#else
    SDL_RWops *rw = SDL_RWFromFile(path, "r");
#endif
    if (!rw) {
        strcpy(path, module_name);
        for (int i = 0; i < len; i++) {
            if (path[i] == '.')
                path[i] = '/';
        }
        strcat(path, "/init.lua");

#if defined(SELENE_USE_SDL3)
        rw = SDL_IOFromFile(path, "r");
#else
        rw = SDL_RWFromFile(path, "r");
#endif
        if (!rw) {
            lua_pushfstring(L, "[selene] RWops: failed to open module: %s", module_name);
            return 1;
        }
    }
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetIOSize(rw);
#else
    size_t size = SDL_RWsize(rw);
#endif
    char *content = malloc(size);
#if defined(SELENE_USE_SDL3)
    SDL_ReadIO(rw, content, size);
#else
    SDL_RWread(rw, content, 1, size);
#endif
    if (luaL_loadbuffer(L, content, size, path) != LUA_OK) {
        free((void *)content);
#if defined(SELENE_USE_SDL3)
        SDL_CloseIO(rw);
#else
        SDL_RWclose(rw);
#endif
        lua_pushfstring(L, "[selene] failed to parse Lua file: %s", path);
        return 1;
    }
    free((void *)content);
#if defined(SELENE_USE_SDL3)
    SDL_CloseIO(rw);
#else
    SDL_RWclose(rw);
#endif
    return 1;
}
#endif

static int l_selene_set_running(lua_State *L) {
    INIT_ARG();
    CHECK_BOOLEAN(running);
    s_ctx->is_running = running;
    return 0;
}

static int l_selene_set_event(lua_State *L) {
    INIT_ARG();
    if (!lua_isfunction(L, arg))
        return luaL_argerror(L, arg, "must be a function");
    lua_pushvalue(L, arg);
    if (s_ctx->l_event_callback_ref == LUA_NOREF)
        s_ctx->l_event_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else
        lua_rawseti(L, LUA_REGISTRYINDEX, s_ctx->l_event_callback_ref);
    return 0;
}

static int l_selene_set_step(lua_State *L) {
    INIT_ARG();
    if (!lua_isfunction(L, arg))
        return luaL_argerror(L, arg, "must be a function");
    lua_pushvalue(L, arg);
    if (s_ctx->l_step_callback_ref == LUA_NOREF)
        s_ctx->l_step_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else
        lua_rawseti(L, LUA_REGISTRYINDEX, s_ctx->l_step_callback_ref);
    return 0;
}

static int l_selene_set_quit(lua_State *L) {
    INIT_ARG();
    if (lua_type(L, arg) != LUA_TFUNCTION)
        return luaL_argerror(L, arg, "must be a function");
    lua_pushvalue(L, arg);
    if (s_ctx->l_quit_callback_ref == LUA_NOREF)
        s_ctx->l_quit_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else
        lua_rawseti(L, LUA_REGISTRYINDEX, s_ctx->l_quit_callback_ref);
    return 0;
}

extern int selene_open_enums(lua_State *L);

/**
 * Alloc memory
 */
static int l_selene_alloc(lua_State* L) {
    int size = (int)luaL_checkinteger(L, 1);
    lua_newuserdata(L, size);
    return 1;
}

/**
 * Timer
 */

static int l_selene_delay(lua_State* L) {
    Uint32 ms = (Uint32)luaL_checkinteger(L, 1);
    SDL_Delay(ms);
    return 0;
}

static int l_selene_get_ticks(lua_State* L) {
    lua_pushinteger(L, SDL_GetTicks());
    return 1;
}

/**
 * Window
 */

static int l_selene_create_window(lua_State* L) {
    INIT_ARG();
    CHECK_STRING(title);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int x = SDL_WINDOWPOS_CENTERED;
    int y = SDL_WINDOWPOS_CENTERED;
    int flags = SDL_WINDOW_SHOWN;
    if (lua_istable(L, arg)) {
        lua_getfield(L, arg, "x");
        x = (int)luaL_optinteger(L, -1, x);
        lua_pop(L, 1);

        lua_getfield(L, arg, "y");
        y = (int)luaL_optinteger(L, -1, y);
        lua_pop(L, 1);

        if (lua_getfield(L, arg, "resizable") == LUA_TBOOLEAN) flags |= SDL_WINDOW_RESIZABLE * lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "always_on_top") == LUA_TBOOLEAN) flags |= SDL_WINDOW_ALWAYS_ON_TOP * lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "borderless") == LUA_TBOOLEAN) flags |= SDL_WINDOW_BORDERLESS * lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (lua_getfield(L, arg, "opengl") == LUA_TBOOLEAN) flags |= SDL_WINDOW_OPENGL * lua_toboolean(L, -1);
        lua_pop(L, 2);
    }
    if (flags & SDL_WINDOW_OPENGL) {
#if defined(OS_EMSCRIPTEN) || defined(OS_ANDROID)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    }
    SDL_Window *win = SDL_CreateWindow(
        title,
#if !defined(SELENE_USE_SDL3)
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
#endif
        width, height, flags
    );
    if (win == NULL)
        return luaL_error(L, "failed to create window: %s", SDL_GetError());
    SDL_Window **win_ptr = (SDL_Window **)lua_newuserdata(L, sizeof(SDL_Window *));
    luaL_setmetatable(L, "sdlWindow");
    *win_ptr = win;
    return 1;
}

static int l_selene_create_renderer(lua_State* L) {
    return l_renderer_create(L);
}

extern int l_AudioSystem_create(lua_State* L);
static int l_selene_create_audio_system(lua_State* L) {
    return l_AudioSystem_create(L);
}

#if 0
static int l_selene_get_context(lua_State* L) {
    lua_newtable(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_ctx->l_window_ref);
    lua_setfield(L, -2, "window");
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_ctx->l_renderer_ref);
    lua_setfield(L, -2, "renderer");
    lua_rawgeti(L, LUA_REGISTRYINDEX, s_ctx->l_audio_system_ref);
    lua_setfield(L, -2, "audio_system");
    return 1;
}
#endif

/**
 * Open and setup the selene module
 * @param L Lua context
 * @return Selene library
 */
int luaopen_selene(lua_State *L) {
    const luaL_Reg reg[] = {
        REG_FIELD(selene, alloc),
        REG_FIELD(selene, delay),
        REG_FIELD(selene, get_ticks),
        /* Runner functions */
        REG_FIELD(selene, set_running),
        // Set callbacks
        REG_FIELD(selene, set_event),
        REG_FIELD(selene, set_step),
        REG_FIELD(selene, set_quit),
        // Get data
        REG_FIELD(selene, create_window),
        REG_FIELD(selene, create_renderer),
        REG_FIELD(selene, create_audio_system),
        // REG_FIELD(selene, get_context),
        {NULL, NULL}
    };
    luaL_newlib(L, reg);
    selene_open_enums(L);
    lua_pushstring(L, SELENE_VERSION);
    lua_setfield(L, -2, "__version");

    /* Load selene internal modules */
    int i;
    for (i = 0; _selene_modules_reg[i].name != NULL; i++) {
        _selene_modules_reg[i].func(L);
        lua_setfield(L, -2, _selene_modules_reg[i].name);
    }

#ifndef OS_EMSCRIPTEN
    lua_pushstring(L, SDL_GetBasePath());
#else
    lua_pushstring(L, "./");
#endif
    lua_setfield(L, -2, "__exec");
    lua_pushstring(L, "./");
    lua_setfield(L, -2, "__dir");

    /* Setup SDL_RWops loader */
    lua_getglobal(L, "package");
#if SELENE_USE_JIT
    lua_getfield(L, -1, "loaders");
#else
    lua_getfield(L, -1, "searchers");
#endif
    lua_pushcfunction(L, l_load_from_sdl_rwops);
    lua_rawseti(L, -2, (lua_Integer)lua_rawlen(L, -2) + 1);
    lua_pop(L, 2);

    /* Setup extended libs */
    l_setup_extended_libs(L);
    /* Setup global modules */
    for (i = 0; _global_modules_reg[i].name != NULL; i++) {
        luaL_requiref(L, _global_modules_reg[i].name, _global_modules_reg[i].func, 1);
        lua_pop(L, 1);
    }

#if 1
    /* Setup call function */
    lua_newtable(L);
    lua_pushcfunction(L, l_selene__call);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
#endif

    return 1;
}

int s_default_event_callback(lua_State *L) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        lua_pushvalue(L, -1);
        // selene_event(L, &event);
    }
    return 0;
}

int s_default_step_callback(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_step_callback_ref);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "[selene] failed to run step function: %s",
                    lua_tostring(L, -1));
        return SELENE_APP_FAILURE;
    }
    return g_selene_context.is_running ? SELENE_APP_CONTINUE : SELENE_APP_SUCCESS;
}

void s_default_quit_callback(lua_State* L, int status) {
#if DEBUG
    fprintf(stdout, "quit: %d\n", status);
#endif
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_quit_callback_ref);
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "[selene] failed to run quit function: %s",
                        lua_tostring(L, -1));
        }
    } else lua_pop(L, 1);
}
