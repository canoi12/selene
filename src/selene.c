#include "selene.h"
#include "lua_helper.h"

#include "modules/renderer.h"
#include "modules/filesystem.h"

int g_sdl_modules = 0;

extern int g_init_renderer(lua_State* L, selene_Renderer* r, SDL_Window* win);

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
};
SeleneContext *s_ctx = &g_selene_context;

static int l_selene__call(lua_State *L) {
    INIT_ARG();
    arg++;
#if 0
    const char *name = luaL_checkstring(L, 2);
    const char *version = luaL_checkstring(L, 3);
    const char *org = luaL_checkstring(L, 4);
#endif
    const char* org = "selene";
    const char* name = "app";
    int flags = 0;
    if (!lua_istable(L, arg)) {
        flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER;
    }
    else {
        int len = lua_rawlen(L, arg);
        for (int i = 0; i < len; i++) {
            lua_rawgeti(L, arg, i+1);
            int opt = luaL_checkoption(L, -1, NULL, s_sdl_init_flags_options);
            flags |= s_sdl_init_flags_values[opt];
            lua_pop(L, 1);
        }
        lua_getfield(L, arg, "meta");
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "org");
            if (lua_isstring(L, -1)) org = lua_tostring(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, -1, "name");
            if (lua_isstring(L, -1)) name = lua_tostring(L, -1);
            lua_pop(L, 1);
            DEBUG_LOG("Meta: %s %s\n", org, name);
        }
        lua_pop(L, 1);
    }

    if (SDL_Init(flags) < 0) {
        return luaL_error(L, "failed to initialize SDL: %s", SDL_GetError());
    }

    char* path = SDL_GetPrefPath(org, name);
    int len = strlen(path);
    if (path[len - 1] == '/' || path[len - 1] == '\\')
        path[len - 1] = '\0';
    lua_pushstring(L, path);
    lua_rawseti(L, LUA_REGISTRYINDEX, r_user_path);
    SDL_free(path);

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

// Selene Modules
extern int luaopen_audio(lua_State *L);
extern int luaopen_filesystem(lua_State* L);
extern int luaopen_font(lua_State* L);
extern int luaopen_json(lua_State* L);
extern int luaopen_renderer(lua_State *L);
extern int luaopen_window(lua_State* L);

static const luaL_Reg _selene_modules_reg[] = {
    {"audio", luaopen_audio},
    {"filesystem", luaopen_filesystem},
    {"font", luaopen_font},
    {"json", luaopen_json},
    {"renderer", luaopen_renderer},
    {"window", luaopen_window},
    {NULL, NULL}
};

#if !defined(SELENE_NO_SDL) && 0
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

extern int l_create_window(lua_State* L);
static int l_selene_create_window(lua_State* L) {
    return l_create_window(L);
}

extern int l_renderer_create(lua_State* L);
static int l_selene_create_renderer(lua_State* L) {
    return l_renderer_create(L);
}

extern int l_AudioSystem_create(lua_State* L);
static int l_selene_create_audio_system(lua_State* L) {
    return l_AudioSystem_create(L);
}

#if defined(OS_ANDROID)
extern int lua_android_require(lua_State* L);
#endif

/**
 * Open and setup the selene module
 * @param L Lua context
 * @return Selene library
 */
int luaopen_selene(lua_State *L) {
    DEBUG_LOG("[selene] luaopen_selene\n");
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
    // selene_open_enums(L);
    DEBUG_LOG("[selene] created newlib\n");
    lua_pushstring(L, SELENE_VERSION);
    lua_setfield(L, -2, "__version");

    /* Load selene internal modules */
    int i;
    for (i = 0; _selene_modules_reg[i].name != NULL; i++) {
        const luaL_Reg* _reg = _selene_modules_reg + i;
        DEBUG_LOG("[selene] loading %s lib\n", _reg->name);
#if 1
        luaL_requiref(L, _reg->name, _reg->func, 1);
        lua_pop(L, 1);
#else
        _reg->func(L);
        lua_setfield(L, -2, _reg->name);
#endif
    }

#if defined(OS_ANDROID)
    /* Setup SDL_RWops loader */
    lua_getglobal(L, "package");
#if SELENE_USE_JIT
    lua_getfield(L, -1, "loaders");
#else
    lua_getfield(L, -1, "searchers");
#endif
    //lua_pushcfunction(L, l_load_from_sdl_rwops);
    lua_pushcfunction(L, lua_android_require);
    //lua_rawseti(L, -2, (lua_Integer)lua_rawlen(L, -2) + 1);
    lua_rawseti(L, -2, 2);
    lua_pop(L, 2);
#endif

    /* Setup extended libs */
    l_setup_extended_libs(L);

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
    DEBUG_LOG("[selene] quit callback: %d\n", status);
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_selene_context.l_quit_callback_ref);
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "[selene] failed to run quit function: %s",
                        lua_tostring(L, -1));
        }
    } else lua_pop(L, 1);
}

int utf8_codepoint(uint8_t* p, int* codepoint) {
    //uint8_t* n = NULL;
    int n = 0;
    uint8_t c = *p;
    if (c < 0x80) { // Se for um ascii normal, o codepoint é o próprio caractere
        *codepoint = c;
        return 1;
    }

    switch (c & 0xf0) { // Checa o primeiro byte para conferir quantos bytes precisam ser usados
    case 0xf0: { // 4 bytes
        *codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | ((p[3] & 0x3f));
        //n = (uint8_t*)p + 4;
        n = 4;
        break;
    }
    case 0xe0: { // 3 bytes
        *codepoint = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | ((p[2] & 0x3f));
        //n = (uint8_t*)p + 3;
        n = 3;
        break;
    }
    case 0xc0:
    case 0xd0: { // 2 bytes
        *codepoint = ((p[0] & 0x1f) << 6) | ((p[1] & 0x3f));
        //n = (uint8_t*)p + 2;
        n = 2;
        break;
    }
    default:
    {
        *codepoint = -1;
        n = 1;
        //n = n + 1;
    }
    }
    if (*codepoint > SELENE_MAX_UNICODE) *codepoint = -1;
    return n;
}

void char_rect(selene_FontGlyph* glyphs, const int c, float *x, float *y, int* out_pos, int* out_rect, int width, int line_height) {
    if (c == '\n') {
        *x = 0;
        *y += line_height;
        return;
    }

    if (c == '\t') {
        *x += glyphs[c].bw * 2;
        return;
    }
    if (width != 0 && *x + (glyphs[c].bl) > width) {
        *x = 0;
        *y += line_height;
    }

    float x2 = *x + (float)glyphs[c].bl;
    float y2 = *y + (float)glyphs[c].bt;

    float s0 = glyphs[c].tx;
    float t0 = 0;
    int s1 = glyphs[c].bw;
    int t1 = glyphs[c].bh;

    *x += glyphs[c].ax;
    *y += glyphs[c].ay;

    if (out_pos) {
        out_pos[0] = x2;
        out_pos[1] = y2;
    }
    if (out_rect) {
        out_rect[0] = s0;
        out_rect[1] = t0;
        out_rect[2] = s1;
        out_rect[3] = t1;
    }
}
