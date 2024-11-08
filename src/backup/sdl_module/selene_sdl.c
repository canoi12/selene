#include "selene_sdl.h"

static SDL_Texture* texture_from_image_data(SDL_Renderer* renderer, int access, ImageData* img_data) {
    int w = img_data->width;
    int h = img_data->height;
    int format = 0;
    switch (img_data->pixel_format) {
        case SELENE_PIXEL_BGR: format = SDL_PIXELFORMAT_BGR888; break;
        case SELENE_PIXEL_RGB: format = SDL_PIXELFORMAT_RGB888; break;
        case SELENE_PIXEL_RGBA: format = SDL_PIXELFORMAT_RGBA32; break;
        case SELENE_PIXEL_BGRA: format = SDL_PIXELFORMAT_BGRA32; break;
    }
    return SDL_CreateTexture(renderer, format, access, w, h);
}

extern MODULE_FUNCTION(sdlAudioDeviceID, meta);
extern MODULE_FUNCTION(sdlAudioStream, meta);
extern MODULE_FUNCTION(sdlEvent, meta);
extern MODULE_FUNCTION(sdlGamepad, meta);
extern MODULE_FUNCTION(sdlGLContext, meta);
extern MODULE_FUNCTION(sdlJoystick, meta);
extern MODULE_FUNCTION(sdlRenderer, meta);
extern MODULE_FUNCTION(sdlRWops, meta);
extern MODULE_FUNCTION(sdlTexture, meta);
extern MODULE_FUNCTION(sdlWindow, meta);

static const luaL_Reg _sdl_meta_regs[] = {
    {"AudioDeviceID", l_sdlAudioDeviceID_meta},
    {"AudioStream", l_sdlAudioStream_meta},
    {"Event", l_sdlEvent_meta},
    {"Gamepad", l_sdlGamepad_meta},
    {"GLContext", l_sdlGLContext_meta},
    {"Joystick", l_sdlJoystick_meta},
    {"Renderer", l_sdlRenderer_meta},
    {"RWops", l_sdlRWops_meta},
    {"Texture", l_sdlTexture_meta},
    {"Window", l_sdlWindow_meta},
};

/************************
 #                      #
 #         Core         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, init) {
    INIT_ARG();
    CHECK_NUMBER(unsigned int, flags);
    int err = SDL_Init(flags);
    PUSH_BOOLEAN(err == 0);
    return 1;
}

static MODULE_FUNCTION(sdl, quit) {
    SDL_Quit();
	return 0;
}

static MODULE_FUNCTION(sdl, is_android_tv) {
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 8)
    PUSH_BOOLEAN(SDL_IsAndroidTV() == 1);
    return 1;
#else
    return 0;
#endif
}

static MODULE_FUNCTION(sdl, is_chromebook) {
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsChromebook() == 1);
    return 1;
#else
    return 0;
#endif
}

static MODULE_FUNCTION(sdl, is_dex_mode) {
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsDeXMode() == 1);
    return 1;
#else
    return 0;
#endif
}

static MODULE_FUNCTION(sdl, is_tablet) {
    PUSH_BOOLEAN(SDL_IsTablet() == 1);
    return 1;
}

/************************
 #                      #
 #     AudioDeviceID    #
 #                      #
 ************************/

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
	return 1;
}

static MODULE_FUNCTION(sdl, gen_num_audio_devices) {
    INIT_ARG();
	GET_BOOLEAN(is_capture);
	PUSH_INTEGER(SDL_GetNumAudioDevices(is_capture));
	return 1;
}

static MODULE_FUNCTION(sdl, get_audio_device_name) {
    INIT_ARG();
	CHECK_INTEGER(index);
	GET_BOOLEAN(is_capture);
	PUSH_STRING	(SDL_GetAudioDeviceName(index, is_capture));
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
    CHECK_UDATA(FontData, font_data);
    ImageData* imgd = (ImageData*)font_data;
    NEW_UDATA(sdlFont, font);
    NEW_UDATA(sdlTexture, tex);
    // lua_rawsetp(L, LUA_REGISTRYINDEX, font);
    font->texture = tex;
    font->font_data = font_data;
    *tex = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, imgd->width, imgd->height);
    if (*tex == NULL)
        return luaL_error(L, "[selene] failed to create SDL_Texture for Font");
    SDL_UpdateTexture(*tex, NULL, imgd->pixels, imgd->width * imgd->channels);
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
    CHECK_INTEGER(joy);
    PUSH_BOOLEAN(SDL_IsGameController(joy));
    return 1;
}

static MODULE_FUNCTION(sdl, gamepad_get_axis_from_string) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
	return 1;
}

static MODULE_FUNCTION(sdl, gamepad_get_button_from_string) {
    INIT_ARG();
    CHECK_STRING(str);
    PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
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
	SDL_GLContext* glctx = SDL_GL_CreateContext(*win);
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
    int res = SDL_GL_SetSwapInterval(value);
    PUSH_BOOLEAN(res == 0);
    return 1;
}

static MODULE_FUNCTION(sdl, gl_get_swap_interval) {
    int res = SDL_GL_GetSwapInterval();
    PUSH_INTEGER(res);
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
        lua_pushfstring(L, "failed to open file %s", path);
        return 2;
    }
    NEW_UDATA(sdlRWops, rwops);
    memcpy(rwops, ops, sizeof(SDL_RWops));
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
	TEST_UDATA(ImageData, img);
	SDL_Texture* tex = NULL;
	if (img) {
        OPT_INTEGER(access, SDL_TEXTUREACCESS_STATIC);
        tex = (SDL_Texture*)texture_from_image_data(*render, access, img);
        if (!tex) return luaL_error(L, "[selene] failed to create SDL texture: %s", SDL_GetError());
        int comp = 3;
        if (img->pixel_format == SDL_PIXELFORMAT_RGBA32) comp = 4;
        int pitch = img->width * comp;
        SDL_UpdateTexture(tex, NULL, img->pixels, pitch);
    } else {
        arg--;
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
    PUSH_BOOLEAN(SDL_HasClipboardText());
    return 1;
}

static MODULE_FUNCTION(sdl, set_clipboard_text) {
    INIT_ARG();
    CHECK_STRING(text);
    SDL_SetClipboardText(text);
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

static MODULE_FUNCTION(sdl, has_screen_keyboard_support) {
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
    SDL_StartTextInput();
    return 0;
}

static MODULE_FUNCTION(sdl, stop_text_input) {
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
    PUSH_INTEGER(SDL_GetTicks());
    return 1;
}

static MODULE_FUNCTION(sdl, delay) {
    Uint32 ms = (Uint32)luaL_checknumber(L, 1);
    SDL_Delay(ms);
    return 0;
}

static MODULE_FUNCTION(sdl, get_performance_counter) {
    PUSH_INTEGER(SDL_GetPerformanceCounter());
    return 1;
}

static MODULE_FUNCTION(sdl, get_performance_frequency) {
    PUSH_INTEGER(SDL_GetPerformanceFrequency());
    return 1;
}

/************************
 #                      #
 #        Video         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, enable_screen_saver) {
    SDL_EnableScreenSaver();
    return 0;
}

/************************
 #                      #
 #        Error         #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, get_error) {
    PUSH_STRING(SDL_GetError());
    return 1;
}

static MODULE_FUNCTION(sdl, set_error) {
    INIT_ARG();
    CHECK_STRING(msg);
    #if !defined(OS_ANDROID)
    SDL_SetError("%s", msg);
    #endif
    return 0;
}

/************************
 #                      #
 #        System        #
 #                      #
 ************************/

static MODULE_FUNCTION(sdl, get_platform) {
    PUSH_STRING(SDL_GetPlatform());
    return 1;
}

static MODULE_FUNCTION(sdl, get_cpu_cache_line_size) {
    PUSH_INTEGER(SDL_GetCPUCacheLineSize());
    return 1;
}

static MODULE_FUNCTION(sdl, get_cpu_count) {
    PUSH_INTEGER(SDL_GetCPUCount());
    return 1;
}

static MODULE_FUNCTION(sdl, get_system_ram) {
    PUSH_INTEGER(SDL_GetSystemRAM());
    return 1;
}

int luaopen_sdl(lua_State* L) {
    BEGIN_REG(reg)
        /* Core */
        REG_FIELD(sdl, init),
        REG_FIELD(sdl, quit),
        /* Platforms */
        REG_FIELD(sdl, is_android_tv),
        REG_FIELD(sdl, is_chromebook),
        REG_FIELD(sdl, is_dex_mode),
        REG_FIELD(sdl, is_tablet),
        /* AudioDeviceID */
        REG_FIELD(sdl, open_audio_device),
        REG_FIELD(sdl, gen_num_audio_devices),
        REG_FIELD(sdl, get_audio_device_name),
        /* AudioStream */
        REG_FIELD(sdl, new_audio_stream),
        /* Event */
        REG_FIELD(sdl, create_event),
        /* Font */
        REG_FIELD(sdl, create_font),
        /* Gamepad */
        REG_FIELD(sdl, gamepad_open),
        REG_FIELD(sdl, is_gamepad),
        REG_FIELD(sdl, gamepad_get_axis_from_string),
        REG_FIELD(sdl, gamepad_get_button_from_string),
        /* GLContext */
        REG_FIELD(sdl, create_gl_context),
        REG_FIELD(sdl, gl_make_current),
        REG_FIELD(sdl, get_gl_proc_address_func),
        REG_FIELD(sdl, gl_get_proc_address),
        REG_FIELD(sdl, gl_set_attribute),
        REG_FIELD(sdl, gl_set_swap_interval),
        REG_FIELD(sdl, gl_get_swap_interval),
        /* Joystick */
        REG_FIELD(sdl, joystick_open),
        REG_FIELD(sdl, num_joysticks),
        /* Renderer */
        REG_FIELD(sdl, create_renderer),
        /* RWops */
        REG_FIELD(sdl, load_file),
        REG_FIELD(sdl, read_file),
        REG_FIELD(sdl, rw_from_file),
        /* Texture */
        REG_FIELD(sdl, create_texture),
        /* Window */
        REG_FIELD(sdl, create_window),
        /* Clipboard */
        REG_FIELD(sdl, get_clipboard_text),
        REG_FIELD(sdl, has_clipboard_text),
        REG_FIELD(sdl, set_clipboard_text),
        /* Filesystem */
        REG_FIELD(sdl, get_base_path),
        REG_FIELD(sdl, get_pref_path),
        /* Shared */
        REG_FIELD(sdl, load_object),
        REG_FIELD(sdl, unload_object),
        REG_FIELD(sdl, load_function),
        /* Keyboard */
        REG_FIELD(sdl, get_keyboard_state),
        REG_FIELD(sdl, check_key),
        REG_FIELD(sdl, has_screen_keyboard_support),
        REG_FIELD(sdl, is_screen_keyboard_shown),
        REG_FIELD(sdl, get_scancode_from_name),
        REG_FIELD(sdl, get_scancode_name),
        REG_FIELD(sdl, get_key_from_name),
        REG_FIELD(sdl, get_key_name),
        REG_FIELD(sdl, start_text_input),
        REG_FIELD(sdl, stop_text_input),
        /* Mouse */
        REG_FIELD(sdl, get_mouse_position),
        REG_FIELD(sdl, get_relative_mouse_position),
        REG_FIELD(sdl, is_mouse_down),
        /* Timer */
        REG_FIELD(sdl, get_ticks),
        REG_FIELD(sdl, delay),
        REG_FIELD(sdl, get_performance_counter),
        REG_FIELD(sdl, get_performance_frequency),
        /* Video */
        REG_FIELD(sdl, enable_screen_saver),
        /* Error */
        REG_FIELD(sdl, get_error),
        REG_FIELD(sdl, set_error),
        /* System */
        REG_FIELD(sdl, get_platform),
        REG_FIELD(sdl, get_cpu_cache_line_size),
        REG_FIELD(sdl, get_cpu_count),
        REG_FIELD(sdl, get_system_ram),
    END_REG()
    luaL_newlib(L, reg);

    for (int i = 0; _sdl_meta_regs[i].name != NULL; i++) {
        _sdl_meta_regs[i].func(LUA_STATE_NAME);
        lua_setfield(L, -2, _sdl_meta_regs[i].name);
    }
    
    LOAD_ENUM(sdl_Enums);
    return 1;
}
