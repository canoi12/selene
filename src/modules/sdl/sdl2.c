#include "selene.h"
#include "lua_helper.h"
BEGIN_ENUM(sdl_Enums)
    ENUM_FIELD(INIT_EVERYTHING, SDL_),
    ENUM_FIELD(INIT_SENSOR, SDL_),
    ENUM_FIELD(INIT_AUDIO, SDL_),
    ENUM_FIELD(INIT_VIDEO, SDL_),
    ENUM_FIELD(INIT_JOYSTICK, SDL_),
    ENUM_FIELD(INIT_GAMECONTROLLER, SDL_),
    ENUM_FIELD(INIT_TIMER, SDL_),
    ENUM_FIELD(INIT_EVENTS, SDL_),
    {"AUDIO_S16", AUDIO_S16},
    {"AUDIO_S16SYS", AUDIO_S16SYS},
    {"AUDIO_F32", AUDIO_F32},
    ENUM_FIELD(QUIT, SDL_),
    ENUM_FIELD(APP_TERMINATING, SDL_),
    ENUM_FIELD(APP_LOWMEMORY, SDL_),
    ENUM_FIELD(APP_WILLENTERBACKGROUND, SDL_),
    ENUM_FIELD(APP_DIDENTERBACKGROUND, SDL_),
    ENUM_FIELD(APP_WILLENTERFOREGROUND, SDL_),
    ENUM_FIELD(APP_DIDENTERFOREGROUND, SDL_),
    ENUM_FIELD(LOCALECHANGED, SDL_),
    ENUM_FIELD(DISPLAYEVENT, SDL_),
    ENUM_FIELD(WINDOWEVENT, SDL_),
    ENUM_FIELD(KEYDOWN, SDL_),
    ENUM_FIELD(KEYUP, SDL_),
    ENUM_FIELD(TEXTEDITING, SDL_),
    ENUM_FIELD(TEXTINPUT, SDL_),
    ENUM_FIELD(KEYMAPCHANGED, SDL_),
#if SDL_VERSION_ATLEAST(2, 0, 22)
    ENUM_FIELD(TEXTEDITING_EXT, SDL_),
#endif
    ENUM_FIELD(MOUSEMOTION, SDL_),
    ENUM_FIELD(MOUSEBUTTONDOWN, SDL_),
    ENUM_FIELD(MOUSEBUTTONUP, SDL_),
    ENUM_FIELD(MOUSEWHEEL, SDL_),
    ENUM_FIELD(JOYAXISMOTION, SDL_),
    ENUM_FIELD(JOYBALLMOTION, SDL_),
    ENUM_FIELD(JOYHATMOTION, SDL_),
    ENUM_FIELD(JOYBUTTONDOWN, SDL_),
    ENUM_FIELD(JOYBUTTONUP, SDL_),
    ENUM_FIELD(JOYDEVICEADDED, SDL_),
    ENUM_FIELD(JOYDEVICEREMOVED, SDL_),
#if SDL_VERSION_ATLEAST(2, 24, 0)
    ENUM_FIELD(JOYBATTERYUPDATED, SDL_),
#endif
    ENUM_FIELD(CONTROLLERAXISMOTION, SDL_),
    ENUM_FIELD(CONTROLLERBUTTONDOWN, SDL_),
    ENUM_FIELD(CONTROLLERBUTTONUP, SDL_),
    ENUM_FIELD(CONTROLLERDEVICEADDED, SDL_),
    ENUM_FIELD(CONTROLLERDEVICEREMOVED, SDL_),
    ENUM_FIELD(CONTROLLERDEVICEREMAPPED, SDL_),
    ENUM_FIELD(CONTROLLERTOUCHPADDOWN, SDL_),
    ENUM_FIELD(CONTROLLERTOUCHPADMOTION, SDL_),
    ENUM_FIELD(CONTROLLERTOUCHPADUP, SDL_),
    ENUM_FIELD(CONTROLLERSENSORUPDATE, SDL_),
    ENUM_FIELD(FINGERDOWN, SDL_),
    ENUM_FIELD(FINGERUP, SDL_),
    ENUM_FIELD(FINGERMOTION, SDL_),
    ENUM_FIELD(DOLLARGESTURE, SDL_),
    ENUM_FIELD(DOLLARRECORD, SDL_),
    ENUM_FIELD(MULTIGESTURE, SDL_),
    ENUM_FIELD(CLIPBOARDUPDATE, SDL_),
    ENUM_FIELD(DROPFILE, SDL_),
    ENUM_FIELD(DROPTEXT, SDL_),
    ENUM_FIELD(DROPBEGIN, SDL_),
    ENUM_FIELD(DROPCOMPLETE, SDL_),
    ENUM_FIELD(AUDIODEVICEADDED, SDL_),
    ENUM_FIELD(AUDIODEVICEREMOVED, SDL_),
    ENUM_FIELD(SENSORUPDATE, SDL_),
    ENUM_FIELD(RENDER_TARGETS_RESET, SDL_),
    ENUM_FIELD(RENDER_DEVICE_RESET, SDL_),
    ENUM_FIELD(RENDERER_ACCELERATED, SDL_),
    ENUM_FIELD(RENDERER_TARGETTEXTURE, SDL_),
    ENUM_FIELD(WINDOWEVENT_CLOSE, SDL_),
    ENUM_FIELD(WINDOWEVENT_SIZE_CHANGED, SDL_),
    ENUM_FIELD(WINDOWEVENT_RESIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MOVED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MINIMIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_MAXIMIZED, SDL_),
    ENUM_FIELD(WINDOWEVENT_RESTORED, SDL_),
    ENUM_FIELD(WINDOWEVENT_SHOWN, SDL_),
    ENUM_FIELD(WINDOWEVENT_HIDDEN, SDL_),
    ENUM_FIELD(WINDOWEVENT_ENTER, SDL_),
    ENUM_FIELD(WINDOWEVENT_LEAVE, SDL_),
    ENUM_FIELD(WINDOWEVENT_FOCUS_GAINED, SDL_),
    ENUM_FIELD(WINDOWEVENT_FOCUS_LOST, SDL_),
    ENUM_FIELD(WINDOWEVENT_TAKE_FOCUS, SDL_),
    ENUM_FIELD(WINDOWEVENT_HIT_TEST, SDL_),
    ENUM_FIELD(WINDOW_SHOWN, SDL_),
    ENUM_FIELD(WINDOW_OPENGL, SDL_),
    ENUM_FIELD(WINDOW_RESIZABLE, SDL_),
    ENUM_FIELD(WINDOW_FULLSCREEN, SDL_),
    ENUM_FIELD(WINDOW_FULLSCREEN_DESKTOP, SDL_),
    ENUM_FIELD(WINDOW_BORDERLESS, SDL_),
    ENUM_FIELD(WINDOW_ALWAYS_ON_TOP, SDL_),
    ENUM_FIELD(WINDOWPOS_CENTERED, SDL_),
    ENUM_FIELD(MESSAGEBOX_ERROR, SDL_),
    ENUM_FIELD(MESSAGEBOX_WARNING, SDL_),
    ENUM_FIELD(MESSAGEBOX_INFORMATION, SDL_),
    ENUM_FIELD(GL_DOUBLEBUFFER, SDL_),
    ENUM_FIELD(GL_DEPTH_SIZE, SDL_),
    ENUM_FIELD(GL_STENCIL_SIZE, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_MASK, SDL_),
    ENUM_FIELD(GL_CONTEXT_MAJOR_VERSION, SDL_),
    ENUM_FIELD(GL_CONTEXT_MINOR_VERSION, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_CORE, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_COMPATIBILITY, SDL_),
    ENUM_FIELD(GL_CONTEXT_PROFILE_ES, SDL_),
    ENUM_FIELD(TEXTUREACCESS_STATIC, SDL_),
    ENUM_FIELD(TEXTUREACCESS_STREAMING, SDL_),
    ENUM_FIELD(TEXTUREACCESS_TARGET, SDL_),
    ENUM_FIELD(PIXELFORMAT_UNKNOWN, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGB332, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGB444, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGB555, SDL_),
    ENUM_FIELD(PIXELFORMAT_BGR555, SDL_),
    ENUM_FIELD(PIXELFORMAT_ARGB4444, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGBA4444, SDL_),
    ENUM_FIELD(PIXELFORMAT_ABGR4444, SDL_),
    ENUM_FIELD(PIXELFORMAT_BGRA4444, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGB565, SDL_),
    ENUM_FIELD(PIXELFORMAT_BGR565, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGB24, SDL_),
    ENUM_FIELD(PIXELFORMAT_BGR24, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGB888, SDL_),
    ENUM_FIELD(PIXELFORMAT_BGR888, SDL_),
    ENUM_FIELD(PIXELFORMAT_ARGB8888, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGBA8888, SDL_),
    ENUM_FIELD(PIXELFORMAT_ABGR8888, SDL_),
    ENUM_FIELD(PIXELFORMAT_BGRA8888, SDL_),
    ENUM_FIELD(PIXELFORMAT_RGBA32, SDL_),
    ENUM_FIELD(PIXELFORMAT_ARGB32, SDL_),
    ENUM_FIELD(PIXELFORMAT_BGRA32, SDL_),
    ENUM_FIELD(PIXELFORMAT_ABGR32, SDL_),
    ENUM_FIELD(BLENDMODE_NONE, SDL_),
    ENUM_FIELD(BLENDMODE_BLEND, SDL_),
    ENUM_FIELD(BLENDMODE_ADD, SDL_),
    ENUM_FIELD(BLENDMODE_MOD, SDL_),
    ENUM_FIELD(BLENDMODE_MUL, SDL_),
    ENUM_FIELD(BLENDMODE_INVALID, SDL_),
    ENUM_FIELD(FLIP_NONE, SDL_),
    ENUM_FIELD(FLIP_VERTICAL, SDL_),
    ENUM_FIELD(FLIP_HORIZONTAL, SDL_),
    ENUM_FIELD(ScaleModeNearest, SDL_),
    ENUM_FIELD(ScaleModeLinear, SDL_),
    ENUM_FIELD(ScaleModeBest, SDL_),
END_ENUM()

typedef SDL_AudioDeviceID sdlAudioDeviceID;
extern MODULE_FUNCTION(sdlAudioDeviceID, meta);
typedef SDL_AudioStream* sdlAudioStream;
extern MODULE_FUNCTION(sdlAudioStream, meta);
typedef SDL_Event sdlEvent;
extern MODULE_FUNCTION(sdlEvent, meta);
typedef SDL_GameController* sdlGamepad;
extern MODULE_FUNCTION(sdlGamepad, meta);
typedef SDL_GLContext* sdlGLContext;
extern MODULE_FUNCTION(sdlGLContext, meta);
typedef SDL_Joystick* sdlJoystick;
extern MODULE_FUNCTION(sdlJoystick, meta);
typedef SDL_Renderer* sdlRenderer;
extern MODULE_FUNCTION(sdlRenderer, meta);
typedef SDL_RWops* sdlRWops;
extern MODULE_FUNCTION(sdlRWops, meta);
typedef SDL_Texture* sdlTexture;
extern MODULE_FUNCTION(sdlTexture, meta);
typedef SDL_Window* sdlWindow;
extern MODULE_FUNCTION(sdlWindow, meta);
typedef struct sdlFont sdlFont;
extern MODULE_FUNCTION(sdlFont, meta);

const BEGIN_REG(sdl_MetaTypes)
    REG_FIELD(sdlAudioDeviceID, meta),
    REG_FIELD(sdlAudioStream, meta),
    REG_FIELD(sdlEvent, meta),
    REG_FIELD(sdlGamepad, meta),
    REG_FIELD(sdlGLContext, meta),
    REG_FIELD(sdlJoystick, meta),
    REG_FIELD(sdlRenderer, meta),
    REG_FIELD(sdlRWops, meta),
    REG_FIELD(sdlTexture, meta),
    REG_FIELD(sdlWindow, meta),
    REG_FIELD(sdlFont, meta),
END_REG()


typedef struct {
    int top, count;
    sdlAudioStream* data;
    int* availables;
} AudioStreamPool;

struct sdlFont {
    FontGlyph* glyphs;
    sdlTexture* texture;
    int width, height;
};
    

/************************
 #                      #
 #         Core         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, init) {
    INIT_ARG();
    CHECK_INTEGER(flags);
    PUSH_BOOLEAN(SDL_Init(flags) == 0);
    return 1;
}

static MODULE_FUNCTION(sdl, quit) {
    INIT_ARG();
    SDL_Quit();
    return 0;
}

static MODULE_FUNCTION(sdl, get_error) {
    INIT_ARG();
    PUSH_STRING(SDL_GetError());
    return 1;
}

/* Platforms check */
static MODULE_FUNCTION(sdl, is_android_tv) {
    INIT_ARG();
    #if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 8)
    PUSH_BOOLEAN(SDL_IsAndroidTV() == 1);
#else
    PUSH_BOOLEAN(0);
#endif

    return 1;
}

static MODULE_FUNCTION(sdl, is_chromebook) {
    INIT_ARG();
    #if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsChromebook() == 1);
#else
    PUSH_BOOLEAN(0);
#endif

    return 1;
}

static MODULE_FUNCTION(sdl, is_dex_mode) {
    INIT_ARG();
    #if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsDeXMode() == 1);
#else
    PUSH_BOOLEAN(0);
#endif

    return 1;
}

static MODULE_FUNCTION(sdl, is_tablet) {
    INIT_ARG();
        PUSH_BOOLEAN(SDL_IsTablet() == 1);

    return 1;
}

/************************
 #                      #
 #     AudioDeviceID    #
 #                      #
 ************************/
 
static void _audio_stream_callback(void* userdata, Uint8* stream, int len) {
    #if defined(OS_WIN)
        Uint8 temp[32000];
    #else
        Uint8 temp[len];
    #endif
    SDL_memset(stream, 0, len);
    AudioStreamPool* pool = (AudioStreamPool*)userdata;
    int i = 0;
    while (i < pool->count) {
        if (pool->data[i] != NULL) {
            int result = SDL_AudioStreamGet(pool->data[i], temp, len);
            if (result < 0) {}
            else if (result != len) {
                SDL_memset(temp + result, 0, len - result);
            }
            SDL_MixAudioFormat(stream, temp, AUDIO_S16SYS, len, SDL_MIX_MAXVOLUME);
        }
        i++;
    }
}

static MODULE_FUNCTION(sdl, open_audio_device) {
    INIT_ARG();
	const char* device = NULL;
    if (lua_type(L, arg) == LUA_TSTRING)
        device = lua_tostring(L, arg);
    arg++;
    GET_BOOLEAN(is_capture);
    if (lua_type(L, arg) != LUA_TTABLE)
        return luaL_argerror(L, arg, "Must be a table");

    lua_getfield(L, arg, "sampleRate");
    int freq = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "channels");
    int channels = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "samples");
    int samples = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, arg, "poolSize");
    int pool_size = (int)luaL_optinteger(L, -1, 256);
    lua_pop(L, 1);
    arg++;

    SDL_AudioSpec desired, obtained;
    desired.freq = freq;
    desired.channels = channels;
    desired.samples = samples;
    desired.format = AUDIO_S16SYS;
    desired.callback = _audio_stream_callback;

    AudioStreamPool* pool = malloc(sizeof(*pool));
    desired.userdata = pool;

    pool->count = pool_size;
    pool->data = malloc(pool_size * sizeof(sdlAudioStream));
    memset(pool->data, 0, pool_size * sizeof(sdlAudioStream));
    pool->top = 0;
    pool->availables = malloc(pool_size * sizeof(int));
    for (int i = 0; i < pool_size; i++) {
        pool->availables[i] = pool_size - i - 1;
    }
    pool->top = pool_size;

    GET_BOOLEAN(allowed_changes);
    sdlAudioDeviceID d = SDL_OpenAudioDevice(device, is_capture, &desired, &obtained, allowed_changes);
    if (d == 0) {
        PUSH_NIL();
        return 1;
    } else {
        NEW_UDATA(sdlAudioDeviceID, dev);
        *dev = d;
    }
    PUSH_VALUE(-1);
    PUSH_LUDATA(pool);
    lua_rawset(L, LUA_REGISTRYINDEX);

    lua_createtable(LUA_STATE_NAME, 0, 4);
    PUSH_INTEGER(obtained.channels);
    SET_FIELD(-2, "channels");
    PUSH_INTEGER(obtained.freq);
    SET_FIELD(-2, "sampleRate");
    PUSH_INTEGER(obtained.samples);
    SET_FIELD(-2, "samples");
    PUSH_INTEGER(obtained.format);
    SET_FIELD(-2, "format");
    PUSH_INTEGER(obtained.size);
    SET_FIELD(-2, "size");

	return 2;
}

static MODULE_FUNCTION(sdl, get_num_audio_devices) {
    INIT_ARG();
    GET_BOOLEAN(is_capture);
    PUSH_INTEGER(SDL_GetNumAudioDevices(is_capture));
    return 1;
}

static MODULE_FUNCTION(sdl, get_audio_device_name) {
    INIT_ARG();
    CHECK_INTEGER(index);
    GET_BOOLEAN(is_capture);
    PUSH_STRING(SDL_GetAudioDeviceName(index, is_capture));
    return 1;
}

/************************
 #                      #
 #      AudioStream     #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, new_audio_stream) {
    INIT_ARG();
    SDL_AudioSpec inSpec, outSpec;

    if (lua_type(L, arg) != LUA_TTABLE)
        return luaL_argerror(L, arg, "Must be a table");
    lua_getfield(L, arg, "format");
    inSpec.format = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "channels");
    inSpec.channels = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "sampleRate");
    inSpec.freq = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    arg++;

    if (lua_type(L, arg) != LUA_TTABLE)
        return luaL_argerror(L, arg, "Must be a table");
    lua_getfield(L, arg, "format");
    outSpec.format = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "channels");
    outSpec.channels = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "sampleRate");
    outSpec.freq = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    SDL_AudioStream* s = SDL_NewAudioStream(inSpec.format, inSpec.channels, inSpec.freq, outSpec.format, outSpec.channels, outSpec.freq);
    if (!s)
        return luaL_error(L, "Failed to create audio stream: %s\n", SDL_GetError()); 
    NEW_UDATA(sdlAudioStream, stream);
    *stream = s;
	return 1;
}

/************************
 #                      #
 #        Event         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, create_event) {
    INIT_ARG();
    NEW_UDATA(sdlEvent, ev);
    return 1;
}

/************************
 #                      #
 #        Font          #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, create_font) {
    INIT_ARG();
    CHECK_UDATA(sdlRenderer, renderer);
    if (lua_type(L, arg) != LUA_TTABLE)
        return luaL_argerror(L, arg, "must be a table");
    lua_getfield(L, arg, "data");
    Data* data = lua_touserdata(L, -1);
    lua_getfield(L, arg, "width");
    int width = lua_tointeger(L, -1);
    lua_getfield(L, arg, "height");
    int height = lua_tointeger(L, -1);
    lua_getfield(L, arg, "channels");
    int channels = lua_tointeger(L, -1);
    lua_getfield(L, arg, "glyphs");
    FontGlyph* glyphs = (FontGlyph*)lua_touserdata(L, -1);
    lua_pop(L, 5);
    const char* pixels = (const char*)(&data[1]);
    NEW_UDATA(sdlFont, font);
    NEW_UDATA(sdlTexture, tex);
    // lua_rawsetp(L, LUA_REGISTRYINDEX, font);
    font->texture = tex;
    font->glyphs = glyphs;
    font->width = width;
    font->height = height;
    *tex = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
    if (*tex == NULL)
        return luaL_error(L, "[selene] failed to create SDL_Texture for Font");
    SDL_UpdateTexture(*tex, NULL, pixels, width * channels);
    return 2;
}

/************************
 #                      #
 #       Gamepad        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, gamepad_open) {
    INIT_ARG();
    CHECK_INTEGER(index);
        SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (ctrl == NULL) lua_pushnil(L);
    else {
        NEW_UDATA(sdlGamepad, controller);
        *controller = ctrl;
    }

    return 1;
}

static MODULE_FUNCTION(sdl, is_gamepad) {
    INIT_ARG();
    CHECK_INTEGER(joystick);
    PUSH_BOOLEAN(SDL_IsGameController(joystick));
    return 1;
}

static MODULE_FUNCTION(sdl, gamepad_get_axis_from_string) {
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(name));
    return 1;
}

static MODULE_FUNCTION(sdl, gamepad_get_button_from_string) {
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(name));
    return 1;
}

/************************
 #                      #
 #       GLContext      #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, create_gl_context) {
    INIT_ARG();
	CHECK_UDATA(sdlWindow, win);
    fprintf(stderr, "Window: %p %p\n", win, *win);
	SDL_GLContext glctx = SDL_GL_CreateContext(*win);
    fprintf(stderr, "GL Context:  %p\n", glctx);
	if (glctx == NULL)
		return luaL_error(L, "[selene] failed to create GL Context: %s", SDL_GetError());
	NEW_UDATA(sdlGLContext, ctx);
	*ctx = glctx;
	return 1;
}

static MODULE_FUNCTION(sdl, gl_make_current) {
    INIT_ARG();
    CHECK_UDATA(sdlWindow, win);
    CHECK_UDATA(sdlGLContext, ctx);
    SDL_GL_MakeCurrent(*win, *ctx);
    return 0;     
}

static MODULE_FUNCTION(sdl, get_gl_proc_address_func) {
    INIT_ARG();
    PUSH_LUDATA(SDL_GL_GetProcAddress);
    return 1;
}

static MODULE_FUNCTION(sdl, gl_get_proc_address) {
    INIT_ARG();
    CHECK_STRING(name);
        void* proc = SDL_GL_GetProcAddress(name);
	PUSH_LUDATA(proc);

    return 1;
}

static MODULE_FUNCTION(sdl, gl_set_attribute) {
    INIT_ARG();
    CHECK_INTEGER(attr);
    CHECK_INTEGER(value);
    SDL_GL_SetAttribute(attr, value);
    return 0;
}

static MODULE_FUNCTION(sdl, gl_set_swap_interval) {
    INIT_ARG();
    GET_BOOLEAN(value);
    PUSH_BOOLEAN(SDL_GL_SetSwapInterval(value) == 0);
    return 1;
}

static MODULE_FUNCTION(sdl, gl_get_swap_interval) {
    INIT_ARG();
    PUSH_INTEGER(SDL_GL_GetSwapInterval());
    return 1;
}

/************************
 #                      #
 #       Joystick       #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, joystick_open) {
    INIT_ARG();
    CHECK_INTEGER(id);
        SDL_Joystick* j = SDL_JoystickOpen(id);
    if (j) {
        NEW_UDATA(sdlJoystick, joy);
        *joy = j;
    }
    else PUSH_NIL();

    return 1;
}

static MODULE_FUNCTION(sdl, num_joysticks) {
    INIT_ARG();
    PUSH_INTEGER(SDL_NumJoysticks());
    return 1;
}

/************************
 #                      #
 #       Renderer       #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, create_renderer) {
    INIT_ARG();
    CHECK_UDATA(sdlWindow, win);
    OPT_INTEGER(index, -1);
    OPT_INTEGER(flags, SDL_RENDERER_ACCELERATED);
    if (*win == NULL) return luaL_error(L, "[selene] invalid SDL window");
    SDL_Renderer* r = SDL_CreateRenderer(*win, index, flags);
    if (r == NULL) return luaL_error(L, "[selene] failed to create SDL renderer: %s", SDL_GetError());
    NEW_UDATA(sdlRenderer, render);
    *render = r;
    return 1;
}

/************************
 #                      #
 #        RWops         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, load_file) {
    INIT_ARG();
    CHECK_STRING(path);
        size_t size;
    void* data = SDL_LoadFile(path, &size);
    PUSH_LUDATA(data);
    PUSH_INTEGER(size);

    return 2;
}

static MODULE_FUNCTION(sdl, read_file) {
    INIT_ARG();
    CHECK_STRING(path);
        size_t size;
    // char* str = (char*)SDL_LoadFile(path, &size);
#if defined(OS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "selene", "loading %s\n", path);
#endif
    SDL_RWops* rw = SDL_RWFromFile(path, "rb");
    if (!rw)
        return luaL_error(L, "Failed to open file %s", path);
    size = SDL_RWsize(rw);
    char* str = malloc(size+1);
    if (!str)
        return luaL_error(L, "Failed to alloc memory for file: %s", path);
    SDL_RWread(rw, str, 1, size);
    str[size] = '\0';
    SDL_RWclose(rw);
    lua_pushstring(L, str);
    free((void*)str);

    return 1;
}

static MODULE_FUNCTION(sdl, rw_from_file) {
    INIT_ARG();
    CHECK_STRING(path);
    CHECK_STRING(mode);
        SDL_RWops* ops = SDL_RWFromFile(path, mode);
    if (!ops) {
        lua_pushnil(L);
        return 1;
    }
    NEW_UDATA(sdlRWops, rwops);
    *rwops = ops;
    // memcpy(rwops, ops, sizeof(SDL_RWops));

    return 1;
}

/************************
 #                      #
 #       Texture        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, create_texture) {
    INIT_ARG();
	CHECK_UDATA(sdlRenderer, render);
	CHECK_INTEGER(format);
    CHECK_INTEGER(access);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    SDL_Texture* tex = SDL_CreateTexture(*render, format, access, width, height);
    if (!tex) return luaL_error(L, "[selene] failed to create SDL texture: %s", SDL_GetError());
    if (lua_type(L, arg) == LUA_TLIGHTUSERDATA) {
        void* data = lua_touserdata(L, arg);
        int comp = 3;
        if (format == SDL_PIXELFORMAT_RGBA32) comp = 4;
        int pitch = width * comp;
        SDL_UpdateTexture(tex, NULL, data, pitch);
    }
    NEW_UDATA(sdlTexture, t);
    *t = tex;
    return 1;
}

/************************
 #                      #
 #        Window        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, create_window) {
    INIT_ARG();
	CHECK_STRING(title);
	CHECK_INTEGER(x);
	CHECK_INTEGER(y);
	CHECK_INTEGER(width);
	CHECK_INTEGER(height);
	CHECK_INTEGER(flags);
	SDL_Window* win = SDL_CreateWindow(
		title,
		x, y,
		width, height,
		flags
	);
    if (win) {
        NEW_UDATA(sdlWindow, window);
        *window = win;
    } else lua_pushnil(L);
        return 1;
}

/************************
 #                      #
 #      Clipboard       #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, get_clipboard_text) {
    char *text = SDL_GetClipboardText();
    PUSH_STRING(text);
    SDL_free(text);
    return 1;
}

static MODULE_FUNCTION(sdl, has_clipboard_text) {
    INIT_ARG();
    PUSH_BOOLEAN(SDL_HasClipboardText());
    return 1;
}

static MODULE_FUNCTION(sdl, set_clipboard_text) {
    INIT_ARG();
    CHECK_STRING(text);
    SDL_SetClipboardText(luaL_checkstring(L, arg));
    return 0;
}

/************************
 #                      #
 #      Filesystem      #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, get_base_path) {
    char *path = SDL_GetBasePath();
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
}

static MODULE_FUNCTION(sdl, get_pref_path) {
    INIT_ARG();
    CHECK_STRING(org);
    CHECK_STRING(app);
    char *path = SDL_GetPrefPath(org, app);
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
}

/************************
 #                      #
 #        Shared        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, load_object) {
    INIT_ARG();
    CHECK_STRING(sofile);
    void *obj = SDL_LoadObject(sofile);
    PUSH_LUDATA(obj);
    return 1;
}

static MODULE_FUNCTION(sdl, unload_object) {
    INIT_ARG();
    CHECK_LUDATA(void, handle);
    SDL_UnloadObject(handle);
    return 0;
}

static MODULE_FUNCTION(sdl, load_function) {
    INIT_ARG();
    GET_LUDATA(void, handle);
    CHECK_STRING(name);
    void *func = SDL_LoadFunction(handle, name);
    PUSH_LUDATA(func);
    return 1;
}

/************************
 #                      #
 #       Keyboard       #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, get_keyboard_state) {
    void* data = (void*)SDL_GetKeyboardState(NULL);
    PUSH_LUDATA(data);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, data);
    return 1;
}

static MODULE_FUNCTION(sdl, check_key) {
    INIT_ARG();
    CHECK_LUDATA(Uint8, k);
    CHECK_INTEGER(i);
    lua_pushboolean(L, k[i]);
    return 1;
}

static MODULE_FUNCTION(sdl, had_screen_keyboard_support) {
    INIT_ARG();
    PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());
    return 1;
}

static MODULE_FUNCTION(sdl, is_screen_keyboard_shown) {
    INIT_ARG();
    CHECK_UDATA(sdlWindow, win);
    PUSH_BOOLEAN(SDL_IsScreenKeyboardShown(*win));
    return 1;
}

static MODULE_FUNCTION(sdl, get_scancode_from_name) {
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GetScancodeFromName(name));
    return 1;
}

static MODULE_FUNCTION(sdl, get_scancode_name) {
    INIT_ARG();
    CHECK_INTEGER(scancode);
    PUSH_STRING(SDL_GetScancodeName(scancode));
    return 1;
}

static MODULE_FUNCTION(sdl, get_key_from_name) {
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GetKeyFromName(name));
    return 1;
}

static MODULE_FUNCTION(sdl, get_key_name) {
    INIT_ARG();
    CHECK_INTEGER(keycode);
    PUSH_STRING(SDL_GetKeyName(keycode));
    return 1;
}

static MODULE_FUNCTION(sdl, start_text_input) {
    INIT_ARG();
    SDL_StartTextInput();
    return 0;
}

static MODULE_FUNCTION(sdl, stop_text_input) {
    INIT_ARG();
    SDL_StopTextInput();
    return 0;
}

/************************
 #                      #
 #         Mouse        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, get_mouse_position) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
}

static MODULE_FUNCTION(sdl, get_relative_mouse_position) {
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
}

static MODULE_FUNCTION(sdl, is_mouse_down) {
    INIT_ARG();
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
    return 1;
}

/************************
 #                      #
 #        Timer         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, get_ticks) {
    INIT_ARG();
    PUSH_INTEGER(SDL_GetTicks());
    return 1;
}

static MODULE_FUNCTION(sdl, delay) {
    INIT_ARG();
    CHECK_NUMBER(unsigned int, ms);
    SDL_Delay(ms);
    return 0;
}

static MODULE_FUNCTION(sdl, get_performance_counter) {
    INIT_ARG();
    PUSH_INTEGER(SDL_GetPerformanceCounter());
    return 1;
}

static MODULE_FUNCTION(sdl, get_performance_frequency) {
    INIT_ARG();
    PUSH_INTEGER(SDL_GetPerformanceFrequency());
    return 1;
}

static MODULE_FUNCTION(sdl, set_timer) {
    INIT_ARG();
    
    return 0;
}

int luaopen_sdl(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(sdl, init),
        REG_FIELD(sdl, quit),
        REG_FIELD(sdl, get_error),
        REG_FIELD(sdl, is_android_tv),
        REG_FIELD(sdl, is_chromebook),
        REG_FIELD(sdl, is_dex_mode),
        REG_FIELD(sdl, is_tablet),
        REG_FIELD(sdl, open_audio_device),
        REG_FIELD(sdl, get_num_audio_devices),
        REG_FIELD(sdl, get_audio_device_name),
        REG_FIELD(sdl, new_audio_stream),
        REG_FIELD(sdl, create_event),
        REG_FIELD(sdl, create_font),
        REG_FIELD(sdl, gamepad_open),
        REG_FIELD(sdl, is_gamepad),
        REG_FIELD(sdl, gamepad_get_axis_from_string),
        REG_FIELD(sdl, gamepad_get_button_from_string),
        REG_FIELD(sdl, create_gl_context),
        REG_FIELD(sdl, gl_make_current),
        REG_FIELD(sdl, get_gl_proc_address_func),
        REG_FIELD(sdl, gl_get_proc_address),
        REG_FIELD(sdl, gl_set_attribute),
        REG_FIELD(sdl, gl_set_swap_interval),
        REG_FIELD(sdl, gl_get_swap_interval),
        REG_FIELD(sdl, joystick_open),
        REG_FIELD(sdl, num_joysticks),
        REG_FIELD(sdl, create_renderer),
        REG_FIELD(sdl, load_file),
        REG_FIELD(sdl, read_file),
        REG_FIELD(sdl, rw_from_file),
        REG_FIELD(sdl, create_texture),
        REG_FIELD(sdl, create_window),
        REG_FIELD(sdl, get_clipboard_text),
        REG_FIELD(sdl, has_clipboard_text),
        REG_FIELD(sdl, set_clipboard_text),
        REG_FIELD(sdl, get_base_path),
        REG_FIELD(sdl, get_pref_path),
        REG_FIELD(sdl, load_object),
        REG_FIELD(sdl, unload_object),
        REG_FIELD(sdl, load_function),
        REG_FIELD(sdl, get_keyboard_state),
        REG_FIELD(sdl, check_key),
        REG_FIELD(sdl, had_screen_keyboard_support),
        REG_FIELD(sdl, is_screen_keyboard_shown),
        REG_FIELD(sdl, get_scancode_from_name),
        REG_FIELD(sdl, get_scancode_name),
        REG_FIELD(sdl, get_key_from_name),
        REG_FIELD(sdl, get_key_name),
        REG_FIELD(sdl, start_text_input),
        REG_FIELD(sdl, stop_text_input),
        REG_FIELD(sdl, get_mouse_position),
        REG_FIELD(sdl, get_relative_mouse_position),
        REG_FIELD(sdl, is_mouse_down),
        REG_FIELD(sdl, get_ticks),
        REG_FIELD(sdl, delay),
        REG_FIELD(sdl, get_performance_counter),
        REG_FIELD(sdl, get_performance_frequency),
        REG_FIELD(sdl, set_timer),
    END_REG()
    luaL_newlib(L, reg);
    for (int i = 0; sdl_MetaTypes[i].name != NULL; i++) {
        sdl_MetaTypes[i].func(L);
        lua_setfield(L, -2, sdl_MetaTypes[i].name);
    }
    LOAD_ENUM(sdl_Enums);
    return 1;
}


/* sdlAudioDeviceID */

static MODULE_FUNCTION(sdlAudioDeviceID, pause) {
    CHECK_META(sdlAudioDeviceID);
    GET_BOOLEAN(pause);
    SDL_PauseAudioDevice(*self, pause);
    return 0;
}

static MODULE_FUNCTION(sdlAudioDeviceID, close) {
    CHECK_META(sdlAudioDeviceID);
    PUSH_VALUE(-1);
    lua_rawget(L, LUA_REGISTRYINDEX);
    AudioStreamPool* pool = lua_touserdata(L, -1);
    free(pool->data);
    free(pool->availables);
    free(pool);
    lua_pop(L, 1);
    SDL_CloseAudioDevice(*self);
    return 0;
}

MODULE_FUNCTION(sdlAudioDeviceID, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlAudioDeviceID, pause),
        REG_FIELD(sdlAudioDeviceID, close),
    END_REG()
    luaL_newmetatable(L, "sdlAudioDeviceID");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlAudioStream */

static MODULE_FUNCTION(sdlAudioStream, clear) {
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamClear(*self);
    return 0;
}

static MODULE_FUNCTION(sdlAudioStream, free) {
    CHECK_META(sdlAudioStream);
    SDL_FreeAudioStream(*self);
    return 0;
}

static MODULE_FUNCTION(sdlAudioStream, flush) {
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamFlush(*self);
    return 0;
}

static MODULE_FUNCTION(sdlAudioStream, bind) {
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(sdlAudioDeviceID, dev);
    PUSH_VALUE(-1);
    lua_rawget(L, LUA_REGISTRYINDEX);
    AudioStreamPool* pool = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (pool->top <= 0)
        return luaL_error(L, "AudioStreamPool is full\n");
    int index = pool->availables[--(pool->top)];
    pool->data[index] = *self;
    return 0;
}

static MODULE_FUNCTION(sdlAudioStream, unbind) {
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(sdlAudioDeviceID, dev);
    PUSH_VALUE(-1);
    lua_rawget(L, LUA_REGISTRYINDEX);
    AudioStreamPool* pool = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (pool->top >= pool->count)
        return luaL_error(L, "AudioStreamPool is empty\n");
    int i = 0;
    for (i = 0; i < pool->count; i++) {
        if (pool->data[i] == *self)
            break;
    }
    if (i < pool->count) {
        pool->availables[(pool->top)++] = i;
        pool->data[i] = NULL;
    }
    return 0;
}

static MODULE_FUNCTION(sdlAudioStream, put) {
    CHECK_META(sdlAudioStream);
    void* data;
    size_t len;
    int type = lua_type(L, arg);
    switch(type) {
        case LUA_TSTRING: {
            data = (void*)luaL_checklstring(L, arg, &len);
        }
        break;
        case LUA_TLIGHTUSERDATA: {
            data = (void*)lua_touserdata(L, arg++);
            len = (size_t)luaL_checkinteger(L, arg++);
        }
        break;
        case LUA_TUSERDATA: {
            CHECK_UDATA(Data, d);
            data = &d[1];
            len = *d;
        }
        break;
    }
    int res = SDL_AudioStreamPut(*self, data, (int)len);
    PUSH_INTEGER(res);
    return 1;
}

static MODULE_FUNCTION(sdlAudioStream, get) {
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(Data, out);
    OPT_INTEGER(size, out[0]);
    int res = SDL_AudioStreamGet(*self, &out[1], size);
    PUSH_INTEGER(res);
    return 1;
}

static MODULE_FUNCTION(sdlAudioStream, available) {
    CHECK_META(sdlAudioStream);
    int res = SDL_AudioStreamAvailable(*self);
    PUSH_INTEGER(res);
    return 1;
}

MODULE_FUNCTION(sdlAudioStream, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlAudioStream, clear),
        REG_FIELD(sdlAudioStream, free),
        REG_FIELD(sdlAudioStream, flush),
        REG_FIELD(sdlAudioStream, bind),
        REG_FIELD(sdlAudioStream, unbind),
        REG_FIELD(sdlAudioStream, put),
        REG_FIELD(sdlAudioStream, get),
        REG_FIELD(sdlAudioStream, available),
    END_REG()
    luaL_newmetatable(L, "sdlAudioStream");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlEvent */

static MODULE_FUNCTION(sdlEvent, poll) {
    CHECK_META(sdlEvent);
    PUSH_BOOLEAN(SDL_PollEvent(self));
    return 1;
}

static MODULE_FUNCTION(sdlEvent, get_type) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->type);
    return 1;
}

static MODULE_FUNCTION(sdlEvent, window_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->window.event);
    PUSH_INTEGER(self->window.windowID);
    PUSH_INTEGER(self->window.data1);
    PUSH_INTEGER(self->window.data2);
    return 4;
}

static MODULE_FUNCTION(sdlEvent, mouse_motion_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->motion.which);
    PUSH_INTEGER(self->motion.x);
    PUSH_INTEGER(self->motion.y);
    PUSH_INTEGER(self->motion.xrel);
    PUSH_INTEGER(self->motion.yrel);
    PUSH_INTEGER(self->motion.state);
    return 6;
}

static MODULE_FUNCTION(sdlEvent, mouse_button_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->button.which);
    PUSH_INTEGER(self->button.button);
    PUSH_INTEGER(self->button.x);
    PUSH_INTEGER(self->button.y);
    PUSH_INTEGER(self->button.clicks);
    return 5;
}

static MODULE_FUNCTION(sdlEvent, mouse_wheel_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->wheel.which);
    PUSH_INTEGER(self->wheel.x);
    PUSH_INTEGER(self->wheel.y);
    PUSH_BOOLEAN(self->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
    return 4;
}

static MODULE_FUNCTION(sdlEvent, keyboard_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->key.keysym.scancode);
    PUSH_INTEGER(self->key.state == SDL_PRESSED);
    PUSH_BOOLEAN(self->key.repeat);
    PUSH_INTEGER(self->key.keysym.sym);
    PUSH_INTEGER(self->key.keysym.mod);
    return 5;
}

static MODULE_FUNCTION(sdlEvent, audio_device_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->adevice.which);
    return 1;
}

static MODULE_FUNCTION(sdlEvent, gamepad_device_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->cdevice.which);
    return 1;
}

static MODULE_FUNCTION(sdlEvent, gamepad_axis_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->caxis.which);
    PUSH_INTEGER(self->caxis.axis);
    PUSH_INTEGER(self->caxis.value);
    return 3;
}

static MODULE_FUNCTION(sdlEvent, gamepad_button_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->cbutton.which);
    PUSH_INTEGER(self->cbutton.button);
    PUSH_INTEGER(self->cbutton.state);
    return 3;
}

static MODULE_FUNCTION(sdlEvent, joy_device_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jdevice.which);
    return 1;
}

static MODULE_FUNCTION(sdlEvent, joy_axis_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jaxis.which);
    PUSH_INTEGER(self->jaxis.axis);
    PUSH_INTEGER(self->jaxis.value);
    return 3;
}

static MODULE_FUNCTION(sdlEvent, joy_ball_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jball.which);
    PUSH_INTEGER(self->jball.ball);
    PUSH_INTEGER(self->jball.xrel);
    PUSH_INTEGER(self->jball.yrel);
    return 4;
}

static MODULE_FUNCTION(sdlEvent, joy_button_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jbutton.which);
    PUSH_INTEGER(self->jbutton.button);
    PUSH_INTEGER(self->jbutton.state);
    return 3;
}

static MODULE_FUNCTION(sdlEvent, joy_hat_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jhat.which);
    PUSH_INTEGER(self->jhat.hat);
    PUSH_INTEGER(self->jhat.value);
    return 3;
}

static MODULE_FUNCTION(sdlEvent, dollar_gesture_event) {
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->dgesture.touchId);
    PUSH_INTEGER(self->dgesture.gestureId);
    PUSH_INTEGER(self->dgesture.numFingers);
    PUSH_NUMBER(self->dgesture.error);
    PUSH_NUMBER(self->dgesture.x);
    PUSH_NUMBER(self->dgesture.y);
    return 6;
}

static MODULE_FUNCTION(sdlEvent, text_event) {
    CHECK_META(sdlEvent);
    PUSH_STRING(self->text.text);
    return 1;
}

static MODULE_FUNCTION(sdlEvent, text_edit_event) {
    CHECK_META(sdlEvent);
    PUSH_STRING(self->edit.text);
    PUSH_INTEGER(self->edit.start);
    PUSH_INTEGER(self->edit.length);
    return 3;
}

MODULE_FUNCTION(sdlEvent, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlEvent, poll),
        REG_FIELD(sdlEvent, get_type),
        REG_FIELD(sdlEvent, window_event),
        REG_FIELD(sdlEvent, mouse_motion_event),
        REG_FIELD(sdlEvent, mouse_button_event),
        REG_FIELD(sdlEvent, mouse_wheel_event),
        REG_FIELD(sdlEvent, keyboard_event),
        REG_FIELD(sdlEvent, audio_device_event),
        REG_FIELD(sdlEvent, gamepad_device_event),
        REG_FIELD(sdlEvent, gamepad_axis_event),
        REG_FIELD(sdlEvent, gamepad_button_event),
        REG_FIELD(sdlEvent, joy_device_event),
        REG_FIELD(sdlEvent, joy_axis_event),
        REG_FIELD(sdlEvent, joy_ball_event),
        REG_FIELD(sdlEvent, joy_button_event),
        REG_FIELD(sdlEvent, joy_hat_event),
        REG_FIELD(sdlEvent, dollar_gesture_event),
        REG_FIELD(sdlEvent, text_event),
        REG_FIELD(sdlEvent, text_edit_event),
    END_REG()
    luaL_newmetatable(L, "sdlEvent");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlGamepad */

static MODULE_FUNCTION(sdlGamepad, get_name) {
    CHECK_META(sdlGamepad);
    PUSH_STRING(SDL_GameControllerName(*self));
    return 1;
}

static MODULE_FUNCTION(sdlGamepad, get_vendor) {
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetVendor(*self));
    return 1;
}

static MODULE_FUNCTION(sdlGamepad, get_product) {
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProduct(*self));
    return 1;
}

static MODULE_FUNCTION(sdlGamepad, get_product_version) {
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*self));
    return 1;
}

const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};

static MODULE_FUNCTION(sdlGamepad, get_axis) {
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*self, axis));
    return 1;
}

static MODULE_FUNCTION(sdlGamepad, get_button) {
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GameControllerGetButton(*self, button) == 1);
    return 1;
}

static MODULE_FUNCTION(sdlGamepad, rumble) {
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    OPT_INTEGER(freq, 100);
    PUSH_NUMBER(SDL_GameControllerRumble(*self, low, high, freq));
    return 1;
}

const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};

static MODULE_FUNCTION(sdlGamepad, current_power_level) {
    CHECK_META(sdlGamepad);
    SDL_Joystick* j = SDL_GameControllerGetJoystick(*self);
    int level = SDL_JoystickCurrentPowerLevel(j)+1;
    PUSH_STRING(gpad_powerlevels[level]);
    return 1;
}

static MODULE_FUNCTION(sdlGamepad, close) {
    CHECK_META(sdlGamepad);
    if (SDL_GameControllerGetAttached(*self))
        SDL_GameControllerClose(*self);
    return 0;
}

MODULE_FUNCTION(sdlGamepad, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlGamepad, get_name),
        REG_FIELD(sdlGamepad, get_vendor),
        REG_FIELD(sdlGamepad, get_product),
        REG_FIELD(sdlGamepad, get_product_version),
        REG_FIELD(sdlGamepad, get_axis),
        REG_FIELD(sdlGamepad, get_button),
        REG_FIELD(sdlGamepad, rumble),
        REG_FIELD(sdlGamepad, current_power_level),
        REG_FIELD(sdlGamepad, close),
    END_REG()
    luaL_newmetatable(L, "sdlGamepad");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlGLContext */

static MODULE_FUNCTION(sdlGLContext, destroy) {
    CHECK_META(sdlGLContext);
    fprintf(stderr, "%p SDL_GL_DeleteContext(%p)\n", self, *self);
    if (*self) SDL_GL_DeleteContext(*self);
    return 0;
}

MODULE_FUNCTION(sdlGLContext, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlGLContext, destroy),
    END_REG()
    luaL_newmetatable(L, "sdlGLContext");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlJoystick */

static MODULE_FUNCTION(sdlJoystick, get_name) {
    CHECK_META(sdlJoystick);
    PUSH_STRING(SDL_JoystickName(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, get_vendor) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetVendor(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, get_product) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProduct(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, get_product_version) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, get_type) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetType(*self));
    return 1;
}

const char *joy_types[] = {
    [SDL_JOYSTICK_TYPE_UNKNOWN] = "Unknown",
    [SDL_JOYSTICK_TYPE_GAMECONTROLLER] = "GameController",
    [SDL_JOYSTICK_TYPE_WHEEL] = "Wheel",
    [SDL_JOYSTICK_TYPE_ARCADE_STICK] = "Arcadestick",
    [SDL_JOYSTICK_TYPE_FLIGHT_STICK] = "FlightStick",
    [SDL_JOYSTICK_TYPE_DANCE_PAD] = "DancePad",
    [SDL_JOYSTICK_TYPE_GUITAR] = "Guitar",
    [SDL_JOYSTICK_TYPE_DRUM_KIT] = "DrumKit",
    [SDL_JOYSTICK_TYPE_ARCADE_PAD] = "ArcadePad",
    [SDL_JOYSTICK_TYPE_THROTTLE] = "Throttle",
};

static MODULE_FUNCTION(sdlJoystick, get_type_string) {
    CHECK_META(sdlJoystick);
    PUSH_STRING(joy_types[SDL_JoystickGetType(*self)]);
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, num_axes) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumAxes(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, num_balls) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumBalls(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, num_hats) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumHats(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, num_buttons) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumButtons(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, get_axis) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*self, axis));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, get_ball) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*self, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
    return 2;
}

static MODULE_FUNCTION(sdlJoystick, get_hat) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*self, hat));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, get_button) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_JoystickGetButton(*self, button));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, rumble) {
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*self, low, high, freq) == 0);
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, current_power_level) {
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*self));
    return 1;
}

static MODULE_FUNCTION(sdlJoystick, close) {
    CHECK_META(sdlJoystick);
    if (SDL_JoystickGetAttached(*self))
        SDL_JoystickClose(*self);
    return 0;
}

MODULE_FUNCTION(sdlJoystick, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlJoystick, get_name),
        REG_FIELD(sdlJoystick, get_vendor),
        REG_FIELD(sdlJoystick, get_product),
        REG_FIELD(sdlJoystick, get_product_version),
        REG_FIELD(sdlJoystick, get_type),
        REG_FIELD(sdlJoystick, get_type_string),
        REG_FIELD(sdlJoystick, num_axes),
        REG_FIELD(sdlJoystick, num_balls),
        REG_FIELD(sdlJoystick, num_hats),
        REG_FIELD(sdlJoystick, num_buttons),
        REG_FIELD(sdlJoystick, get_axis),
        REG_FIELD(sdlJoystick, get_ball),
        REG_FIELD(sdlJoystick, get_hat),
        REG_FIELD(sdlJoystick, get_button),
        REG_FIELD(sdlJoystick, rumble),
        REG_FIELD(sdlJoystick, current_power_level),
        REG_FIELD(sdlJoystick, close),
    END_REG()
    luaL_newmetatable(L, "sdlJoystick");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlRenderer */

#include "helper.h"

static MODULE_FUNCTION(sdlRenderer, destroy) {
    CHECK_META(sdlRenderer);
    SDL_DestroyRenderer(*self);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, clear) {
    CHECK_META(sdlRenderer);
    SDL_RenderClear(*self);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, set_color) {
    CHECK_META(sdlRenderer);
    Uint8 d[4];
    for (int i = 0; i < 4; i++) d[i] = (Uint8)luaL_checkinteger(L, i+2);
    SDL_SetRenderDrawColor(*self, d[0], d[1], d[2], d[3]);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, set_target) {
    CHECK_META(sdlRenderer);
    const TEST_UDATA(sdlTexture, tex);
    SDL_Texture* t = tex ? *tex : NULL;
    SDL_SetRenderTarget(*self, t);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, set_clip_rect) {
    CHECK_META(sdlRenderer);
    SDL_Rect* psrc = NULL;
    if (lua_type(L, arg) == LUA_TTABLE) {
        int src[4];
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg, i+1);
            src[i] = (int)luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        psrc = (SDL_Rect*)src;
    }
    SDL_RenderSetClipRect(*self, psrc);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, set_blend_mode) {
    CHECK_META(sdlRenderer);
    CHECK_INTEGER(blend_mode);
    SDL_SetRenderDrawBlendMode(*self, blend_mode);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, draw_point) {
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, x);
    const CHECK_NUMBER(float, y);
    SDL_RenderDrawPointF(*self, x, y);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, draw_line) {
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, x0);
    const CHECK_NUMBER(float, y0);
    const CHECK_NUMBER(float, x1);
    const CHECK_NUMBER(float, y1);
    SDL_RenderDrawLineF(*self, x0, y0, x1, y1);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, draw_triangle) {
    CHECK_META(sdlRenderer);
    float pos[6];
    for (int i = 0; i < 6; i++) pos[i] = (float)luaL_checknumber(L, i+2);
    SDL_RenderDrawLineF(*self, pos[0], pos[1], pos[2], pos[3]);
    SDL_RenderDrawLineF(*self, pos[2], pos[3], pos[4], pos[5]);
    SDL_RenderDrawLineF(*self, pos[4], pos[5], pos[0], pos[1]);
    return 0;
}

static void s_vertices_swap(float v0[2], float v1[2]) {
    float aux[2] = {v0[0], v0[1]};
    // printf("V: {%d %d} {%d %d}\n", v0[0], v0[1], v1[0], v1[1]);
    v0[0] = v1[0];
    v0[1] = v1[1];
    v1[0] = aux[0];
    v1[1] = aux[1];
    // *v0 = *v1;
    // *v1 = aux;
}

static void s_vertices_y_sort(float v[3][2]) {
    if (v[1][1] < v[0][1]) s_vertices_swap(v[0], v[1]);
    if (v[2][1] < v[1][1]) s_vertices_swap(v[1], v[2]);
    if (v[2][1] < v[0][1]) s_vertices_swap(v[0], v[2]);
}

static void s_bottom_flat_triangle(SDL_Renderer* r, float v[3][2]) {
    float invslope1 = (float)(v[1][0] - v[0][0]) / (float)(v[1][1] - v[0][1]);
    float invslope2 = (float)(v[2][0] - v[0][0]) / (float)(v[2][1] - v[0][1]);
	
    float curx1, curx2;
    curx1 = curx2 = v[0][0];

    int scanY;
    for (scanY = (int)v[0][1]; scanY <= v[1][1]; scanY++) {
        float v0[2] = {curx1, (float)scanY};
        float v1[2] = {curx2, (float)scanY};
        SDL_RenderDrawLineF(r, v0[0], v0[1], v1[0], v1[1]);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

static void s_top_flat_triangle(SDL_Renderer* r, float v[3][2]) {
    float invslope1 = (v[2][0] - v[0][0]) / (v[2][1] - v[0][1]);
    float invslope2 = (v[2][0] - v[1][0]) / (v[2][1] - v[1][1]);
	
    float curx1, curx2;
    curx1 = curx2 = v[2][0];

    int scanY;
    for (scanY = (int)v[2][1]; scanY >= v[0][1]; scanY--) {
        float v0[2] = {curx1, (float)scanY};
        float v1[2] = {curx2, (float)scanY};
        SDL_RenderDrawLineF(r, v0[0], v0[1], v1[0], v1[1]);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

static MODULE_FUNCTION(sdlRenderer, fill_triangle) {
    CHECK_META(sdlRenderer);
    float v[3][2] = {
        {0.f, 0.f},
        {0.f, 0.f},
        {0.f, 0.f}
    };
    float* p = v[0];
    for (int i = 0; i < 6; i++) p[i] = (float)luaL_checknumber(L, i+2);
    s_vertices_y_sort(v);
    if (v[1][1] == v[2][1]) s_bottom_flat_triangle(*self, v);
    else if (v[0][1] == v[1][1]) s_top_flat_triangle(*self, v);
    else {
        float v3[2];
        v3[0] = v[0][0] + ((v[1][1] - v[0][1]) / (v[2][1] - v[0][1]))  * (v[2][0] - v[0][0]);
        v3[1] = v[1][1];
        float va[3][2] = {
            {v[0][0], v[0][1]},
            {v[1][0], v[1][1]},
            {v3[0], v3[1]}
        };
        float vb[3][2] = {
           {v[1][0], v[1][1]},
            {v3[0], v3[1]},
            {v[2][0], v[2][1]}
        };
        s_bottom_flat_triangle(*self, va);
        s_top_flat_triangle(*self, vb);
    }
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, draw_rect) {
    CHECK_META(sdlRenderer);
    float d[4];
    for (int i = 0; i < 4; i++) {
        d[i] = (float)luaL_checknumber(L, i+2);
    }
    SDL_FRect rect = {d[0], d[1], d[2], d[3]};
    SDL_RenderDrawRectF(*self, &rect);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, fill_rect) {
    CHECK_META(sdlRenderer);
    float d[4];
    for (int i = 0; i < 4; i++) {
        d[i] = (float)luaL_checknumber(L, i+2);
    }
    SDL_FRect rect = {d[0], d[1], d[2], d[3]};
    SDL_RenderFillRectF(*self, &rect);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, draw_circle) {
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, x);
    const CHECK_NUMBER(float, y);
    const CHECK_INTEGER(radius);
    const OPT_INTEGER(sides, 32);

    for (int i = 0; i < sides; i++) {
        float tetha = ((float)i * (float)M_PI2) / (float)sides;
        float p0[2];
        p0[0] = x + (cosf(tetha) * (float)radius);
        p0[1] = y + (sinf(tetha) * (float)radius);

        tetha = ((float)(i+1) * (float)M_PI2) / (float)sides;
        float p1[2];
        p1[0] = x + (cosf(tetha) * (float)radius);
        p1[1] = y + (sinf(tetha) * (float)radius);
        SDL_RenderDrawLineF(*self, p0[0], p0[1], p1[0], p1[1]);
    }
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, fill_circle) {
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, cx);
    const CHECK_NUMBER(float, cy);
    const CHECK_INTEGER(radius);
    int x = radius;
    int y = 0;
    int err = 1 - x;

    while (x >= y) {
        SDL_RenderDrawLineF(*self, cx - x, cy + y, cx + x, cy + y);
        SDL_RenderDrawLineF(*self, cx - y, cy + x, cx + y, cy + x);
        SDL_RenderDrawLineF(*self, cx - x, cy - y, cx + x, cy - y);
        SDL_RenderDrawLineF(*self, cx - y, cy - x, cx + y, cy - x);

        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }

    return 0;
}

static MODULE_FUNCTION(sdlRenderer, copy) {
    CHECK_META(sdlRenderer);
    CHECK_UDATA(sdlTexture, tex);
    SDL_Rect* psrc = NULL;
    SDL_FRect* pdst = NULL;
    int src[4];
    if (lua_type(L, arg) == LUA_TTABLE) {
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg, i+1);
            src[i] = (int)luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        psrc = (SDL_Rect*)src;
    }
    float dst[4];
    if (lua_type(L, arg+1) == LUA_TTABLE) {
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg+1, i+1);
            dst[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        pdst = (SDL_FRect*)dst;
    }
    
    SDL_RenderCopyF(*self, *tex, psrc, pdst);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, copy_ex) {
    CHECK_META(sdlRenderer);
    CHECK_UDATA(sdlTexture, tex);
    SDL_Rect* psrc = NULL;
    SDL_FRect* pdst = NULL;
    int src[4];
    if (lua_type(L, arg) == LUA_TTABLE) {
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg, i+1);
            src[i] = (int)luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        psrc = (SDL_Rect*)src;
    }
    float dst[4];
    if (lua_type(L, arg+1) == LUA_TTABLE) {
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg+1, i+1);
            dst[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        pdst = (SDL_FRect*)dst;
    }
    arg += 2;
    OPT_NUMBER(double, angle, 0.f);
    SDL_FPoint* center = NULL;
    float ctr[2];
    if (lua_type(L, arg) == LUA_TTABLE) {
        for (int i = 0; i < 2; i++) {
            lua_rawgeti(L, arg, i+1);
            ctr[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        center = (SDL_FPoint*)ctr;
    }
    arg += 1;
    OPT_INTEGER(flip, SDL_FLIP_NONE);
    SDL_RenderCopyExF(*self, *tex, psrc, pdst, angle, center, flip);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, print) {
    CHECK_META(sdlRenderer);
    CHECK_UDATA(sdlFont, font);
    CHECK_STRING(text);
    OPT_NUMBER(float, x, 0);
    OPT_NUMBER(float, y, 0);
    float ox = x;
    float oy = y;

    float x0, y0;
    x0 = 0.f;
    y0 = 0.f;
    uint8_t* p = (uint8_t*)text;
    int w = font->width;
    int h = font->height;
    while (*p != 0) {
        int codepoint;
        int n = utf8_codepoint(p, &codepoint);
        if (n > -1) p += n;
        SDL_FRect dest;
        int pos[2];
        int src[4];
        char_rect(font->glyphs, codepoint, &x0, &y0, pos, src, 0, font->height);
        dest.x = x + pos[0];
        dest.y = y + pos[1];
        dest.w = (float)src[2];
        dest.h = (float)src[3];
        // fprintf(stderr, "%c:%d", *p, *p);
        // fprintf(stdout, "src (%c): %d %d %d %d\n", *p, src[0], src[1], src[2], src[3]);

        SDL_Rect t;
        t.x = src[0];
        t.y = src[1];
        t.w = src[2];
        t.h = src[3];
        SDL_RenderCopyF(*self, *(font->texture), &t, &dest);
    }
    // fprintf(stderr, "\n");
    
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, flush) {
    CHECK_META(sdlRenderer);
    SDL_RenderFlush(*self);
    return 0;
}

static MODULE_FUNCTION(sdlRenderer, present) {
    CHECK_META(sdlRenderer);
    SDL_RenderPresent(*self);
    return 0;
}

MODULE_FUNCTION(sdlRenderer, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlRenderer, destroy),
        REG_FIELD(sdlRenderer, clear),
        REG_FIELD(sdlRenderer, set_color),
        REG_FIELD(sdlRenderer, set_target),
        REG_FIELD(sdlRenderer, set_clip_rect),
        REG_FIELD(sdlRenderer, set_blend_mode),
        REG_FIELD(sdlRenderer, draw_point),
        REG_FIELD(sdlRenderer, draw_line),
        REG_FIELD(sdlRenderer, draw_triangle),
        REG_FIELD(sdlRenderer, fill_triangle),
        REG_FIELD(sdlRenderer, draw_rect),
        REG_FIELD(sdlRenderer, fill_rect),
        REG_FIELD(sdlRenderer, draw_circle),
        REG_FIELD(sdlRenderer, fill_circle),
        REG_FIELD(sdlRenderer, copy),
        REG_FIELD(sdlRenderer, copy_ex),
        REG_FIELD(sdlRenderer, print),
        REG_FIELD(sdlRenderer, flush),
        REG_FIELD(sdlRenderer, present),
    END_REG()
    luaL_newmetatable(L, "sdlRenderer");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlRWops */

static MODULE_FUNCTION(sdlRWops, read) {
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(maxnum, 1);
    size_t read = SDL_RWread(*self, data, size, maxnum);
    PUSH_INTEGER(read);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, writen) {
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(num, 1);
    size_t written = SDL_RWwrite(*self, data, size, num);
    PUSH_INTEGER(written);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, seek) {
    CHECK_META(sdlRWops);
    CHECK_INTEGER(offset);
    OPT_INTEGER(whence, RW_SEEK_SET);
    Sint64 res = SDL_RWseek(*self, offset, whence);
    PUSH_INTEGER(res);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, size) {
    CHECK_META(sdlRWops);
    Sint64 size = SDL_RWsize(*self);
    PUSH_INTEGER(size);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, tell) {
    CHECK_META(sdlRWops);
    Sint64 offset = SDL_RWtell(*self);
    PUSH_INTEGER(offset);
    return 1;
}

static MODULE_FUNCTION(sdlRWops, close) {
    CHECK_META(sdlRWops);
    Sint64 res = SDL_RWclose(*self);
    PUSH_BOOLEAN(res == 0);
    return 1;
}

MODULE_FUNCTION(sdlRWops, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlRWops, read),
        REG_FIELD(sdlRWops, writen),
        REG_FIELD(sdlRWops, seek),
        REG_FIELD(sdlRWops, size),
        REG_FIELD(sdlRWops, tell),
        REG_FIELD(sdlRWops, close),
    END_REG()
    luaL_newmetatable(L, "sdlRWops");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlTexture */

static MODULE_FUNCTION(sdlTexture, destroy) {
    CHECK_META(sdlTexture);
    SDL_DestroyTexture(*self);
    return 0;
}

static MODULE_FUNCTION(sdlTexture, query) {
    CHECK_META(sdlTexture);
    Uint32 format;
    int access;
    int w, h;
    SDL_QueryTexture(*self, &format, &access, &w, &h);
    PUSH_INTEGER(format);
    PUSH_INTEGER(access);
    PUSH_INTEGER(w);
    PUSH_INTEGER(h);
    return 4;
}

static MODULE_FUNCTION(sdlTexture, set_alpha_mod) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(alpha);
    SDL_SetTextureAlphaMod(*self, (Uint8)alpha);
    return 0;
}

static MODULE_FUNCTION(sdlTexture, set_color_mod) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(r);
    CHECK_INTEGER(g);
    CHECK_INTEGER(b);
    SDL_SetTextureColorMod(*self, r, g, b);
    return 0;
}

static MODULE_FUNCTION(sdlTexture, set_blend_mode) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(blend_mode);
    SDL_SetTextureBlendMode(*self, blend_mode);
    return 0;
}

static MODULE_FUNCTION(sdlTexture, set_scale_mode) {
    CHECK_META(sdlTexture);
    CHECK_INTEGER(scale_mode);
    SDL_SetTextureScaleMode(*self, scale_mode);
    return 0;
}

MODULE_FUNCTION(sdlTexture, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlTexture, destroy),
        REG_FIELD(sdlTexture, query),
        REG_FIELD(sdlTexture, set_alpha_mod),
        REG_FIELD(sdlTexture, set_color_mod),
        REG_FIELD(sdlTexture, set_blend_mode),
        REG_FIELD(sdlTexture, set_scale_mode),
    END_REG()
    luaL_newmetatable(L, "sdlTexture");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlWindow */

static MODULE_FUNCTION(sdlWindow, destroy) {
    CHECK_META(sdlWindow);
    fprintf(stderr, "sdlWindow: destroy\n");
    if (*self) SDL_DestroyWindow(*self);
    return 0;
}

static MODULE_FUNCTION(sdlWindow, get_size) {
    INIT_ARG();
    CHECK_UDATA(sdlWindow, window);
    int width, height;
    SDL_GetWindowSize(*window, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
    return 2;
}

static MODULE_FUNCTION(sdlWindow, set_size) {
}

static MODULE_FUNCTION(sdlWindow, get_position) {
    CHECK_META(sdlWindow);
    int x, y;
    SDL_GetWindowPosition(*self, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static MODULE_FUNCTION(sdlWindow, set_position) {
}

static MODULE_FUNCTION(sdlWindow, gl_swap) {
    CHECK_META(sdlWindow);
    SDL_GL_SwapWindow(*self);
    return 0;
}

MODULE_FUNCTION(sdlWindow, meta) {
    BEGIN_REG(reg)
        REG_FIELD(sdlWindow, destroy),
        REG_FIELD(sdlWindow, get_size),
        REG_FIELD(sdlWindow, set_size),
        REG_FIELD(sdlWindow, get_position),
        REG_FIELD(sdlWindow, set_position),
        REG_FIELD(sdlWindow, gl_swap),
    END_REG()
    luaL_newmetatable(L, "sdlWindow");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

/* sdlFont */

MODULE_FUNCTION(sdlFont, meta) {
    BEGIN_REG(reg)
    END_REG()
    luaL_newmetatable(L, "sdlFont");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

