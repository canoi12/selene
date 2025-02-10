#include "selene.h"
#include "lua_helper.h"

SeleneContext g_selene_context = {
    .is_running = 0,
    .initialized_sdl = 0,

    .step_callback_ref = LUA_NOREF,
    .quit_callback_ref = LUA_NOREF,
    .event_callback_ref = LUA_NOREF,

    .audio_ref = LUA_NOREF,
    .window_ref = LUA_NOREF,
    .renderer_ref = LUA_NOREF,
};
SeleneContext* s_ctx = &g_selene_context;

extern int l_selene_renderer_Batch2D__call(lua_State* L);

static int l_selene__call(lua_State* L) {
    const char* name = luaL_checkstring(L, 2);
    const char* version = luaL_checkstring(L, 3);
    const char* org = luaL_checkstring(L, 4);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        return luaL_error(L, "failed to initialize SDL: %s", SDL_GetError());
    }
    g_selene_context.initialized_sdl = 1;
    SDL_Window* win = SDL_CreateWindow(
        name,
#if !defined(SELENE_USE_SDL3)
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
#endif
        640, 380,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    if (win == NULL)
        return luaL_error(L, "failed to create window: %s", SDL_GetError());
    SDL_Window** win_ptr = (SDL_Window**)lua_newuserdata(L, sizeof(SDL_Window*));
    luaL_setmetatable(L, "sdlWindow");
    *win_ptr = win;
    g_selene_context.window_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    /*if (luaL_dostring(L, "selene.renderer.Batch2D()") != LUA_OK) {
        return luaL_error(L, "failed to create renderer: %s", SDL_GetError());
    }*/
   l_selene_renderer_Batch2D__call(L);
/*#if defined(SELENE_USE_SDL3)
    SDL_Renderer* r = SDL_CreateRenderer(win, "opengl");
#else
    SDL_Renderer* r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
#endif
    if (r == NULL)
        return luaL_error(L, "failed to create renderer: %s", SDL_GetError());
    SDL_Renderer** r_ptr = (SDL_Renderer**)lua_newuserdata(L, sizeof(SDL_Renderer*));
    luaL_setmetatable(L, "sdlRenderer");
    *r_ptr = r;*/
    g_selene_context.renderer_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

#if SELENE_USE_JIT
void luaL_requiref (lua_State *L, const char *modname,
                               lua_CFunction openf, int glb) {
  lua_pushcfunction(L, openf);
  lua_pushstring(L, modname);  /* argument to open function */
  lua_call(L, 1, 1);  /* open module */
  lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_pushvalue(L, -2);  /* make copy of module (call result) */
  lua_setfield(L, -2, modname);  /* _LOADED[modname] = module */
  lua_pop(L, 1);  /* remove _LOADED table */
  if (glb) {
    lua_pushvalue(L, -1);  /* copy of 'mod' */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}
#endif

extern int l_setup_extended_libs(lua_State* L);

// Type Modules
extern int luaopen_Data(lua_State *L);

// Library Modules
#ifndef SELENE_NO_AUDIO
extern int luaopen_audio(lua_State* L);
#endif
extern int luaopen_fs(lua_State *L);
extern int luaopen_gl(lua_State *L);
extern int luaopen_linmath(lua_State *L);
#ifndef SELENE_NO_JSON
extern int luaopen_json(lua_State* L);
#endif
#ifndef SELENE_NO_IMAGE
extern int luaopen_image(lua_State* L);
#endif
#ifndef SELENE_NO_FONT
extern int luaopen_font(lua_State* L);
#endif
#ifndef SELENE_NO_SDL
extern int luaopen_sdl(lua_State *L);
#endif

luaL_Reg _mod_regs[] = {
#ifndef SELENE_NO_AUDIO
    {"audio", luaopen_audio},
#endif
    {"fs", luaopen_fs},
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
    // {"sdl", luaopen_sdl},
#endif
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
    SDL_IOStream* rw = SDL_IOFromFile(path, "r");
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

static int l_selene_set_running(lua_State* L) {
    INIT_ARG();
    CHECK_BOOLEAN(running);
    s_ctx->is_running = running;
    return 0;
}

static int l_selene_set_event(lua_State* L) {
    INIT_ARG();
    if (!lua_isfunction(L, arg))
        return luaL_argerror(L, arg, "must be a function");
    lua_pushvalue(L, arg);
    if (s_ctx->event_callback_ref == LUA_NOREF) s_ctx->event_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_rawseti(L, LUA_REGISTRYINDEX, s_ctx->event_callback_ref);
    return 0;
}

static int l_selene_set_step(lua_State* L) {
    INIT_ARG();
    if (!lua_isfunction(L, arg))
        return luaL_argerror(L, arg, "must be a function");
    lua_pushvalue(L, arg);
    if (s_ctx->step_callback_ref == LUA_NOREF) s_ctx->step_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_rawseti(L, LUA_REGISTRYINDEX, s_ctx->step_callback_ref);
    return 0;
}

static int l_selene_set_quit(lua_State* L) {
    INIT_ARG();
    if (lua_type(L, arg) != LUA_TFUNCTION)
        return luaL_argerror(L, arg, "must be a function");
    lua_pushvalue(L, arg);
    if (s_ctx->quit_callback_ref == LUA_NOREF) s_ctx->quit_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_rawseti(L, LUA_REGISTRYINDEX, s_ctx->quit_callback_ref);
    return 0;
}

extern int luaopen_renderer(lua_State* L);

/**
 * Open and setup the selene module
 * @param L Lua context
 * @return Selene library
 */
int luaopen_selene(lua_State *L) {
    luaL_Reg reg[] = {
      /* Runner functions */
      {"set_running", l_selene_set_running},
      {"set_event", l_selene_set_event},
      {"set_step", l_selene_set_step},
      {"set_quit", l_selene_set_quit},
      {NULL, NULL}
    };
    luaL_newlib(L, reg);
    // LOAD_MODULE(AudioDecoder);
    lua_pushstring(L, SELENE_VERSION);
    lua_setfield(L, -2, "__version");
    LOAD_MODULE(Data);
    luaL_requiref(L, "renderer", luaopen_renderer, 0);
    lua_setfield(L, -2, "renderer");

#ifndef OS_EMSCRIPTEN
    lua_pushstring(L, SDL_GetBasePath());
#else
    lua_pushstring(L, "./");
#endif
    lua_setfield(L, -2, "__exec");
    lua_pushstring(L, "./");
    lua_setfield(L, -2, "__dir");


    lua_getglobal(L, "package");
#if SELENE_USE_JIT
    lua_getfield(L, -1, "loaders");
#else
    lua_getfield(L, -1, "searchers");
#endif
    lua_pushcfunction(L, l_load_from_sdl_rwops);
    lua_rawseti(L, -2, (lua_Integer)lua_rawlen(L, -2) + 1);
    lua_pop(L, 2);


    l_setup_extended_libs(L);
    int i;
    for (i = 0; _mod_regs[i].name != NULL; i++) {
        luaL_requiref(L, _mod_regs[i].name, _mod_regs[i].func, 1);
        lua_pop(L, 1);
    }

    lua_newtable(L);
    lua_pushcfunction(L, l_selene__call);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);

    return 1;
}