--- @type generator.Group
local Core = {
    c_code_block(
[[
/************************
 #                      #
 #         Core         #
 #                      #
 ************************/
]]
    ),
    function_block('boolean', 'init', 'integer flags', { ccall = "PUSH_BOOLEAN(SDL_Init(flags) == 0);" }),
    function_block('', 'quit', '', { ccall = "SDL_Quit();" }),
}

--- @type generator.Group
local Platforms = {
    c_code_block("/* Platforms check */"),
    function_block('boolean', 'is_android_tv', '',
    {
        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 8)
    PUSH_BOOLEAN(SDL_IsAndroidTV() == 1);
#else
    PUSH_BOOLEAN(0);
#endif
]]
    }),
    function_block('boolean', 'is_chromebook', '',
    {
        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsChromebook() == 1);
#else
    PUSH_BOOLEAN(0);
#endif
]]
    }),
    function_block('boolean', 'is_dex_mode', '',
    {
        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsDeXMode() == 1);
#else
    PUSH_BOOLEAN(0);
#endif
]]
    }),
    function_block('boolean', 'is_tablet', '',
    {
        ccall =
[[
    PUSH_BOOLEAN(SDL_IsTablet() == 1);
]]
    }),
}

local AudioDeviceID = {
    c_code_block(
[[
/************************
 #                      #
 #     AudioDeviceID    #
 #                      #
 ************************/
 ]]
    ),
    c_code_block(
[[
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
]]
    ),
    function_block('sdlAudioDeviceID', 'open_audio_device', 'string device, boolean is_capture, table specs',
    {
        cdef =
[[
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
]]
    }),
    function_block('integer', 'get_num_audio_devices', 'boolean is_capture', { ccall = "PUSH_INTEGER(SDL_GetNumAudioDevices(is_capture));" }),
    function_block('integer', 'get_audio_device_name', 'integer index, boolean is_capture', { ccall = "PUSH_STRING(SDL_GetAudioDeviceName(index, is_capture));" }),
}

local AudioStream = {
    c_code_block(
[[
/************************
 #                      #
 #      AudioStream     #
 #                      #
 ************************/
]]
    ),
    function_block('sdlAudioStream', 'new_audio_stream', 'table in_spec, table out_spec',
    {
        cdef =
[[
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
]]
    })
}

local Event = {
    c_code_block(
[[
/************************
 #                      #
 #        Event         #
 #                      #
 ************************/
]]
    ),
    function_block('sdlEvent', 'create_event', '', { ccall = "NEW_UDATA(sdlEvent, ev);" })
}

local Font = {
    c_code_block(
[[
/************************
 #                      #
 #        Font          #
 #                      #
 ************************/
]]
    ),
    function_block('sdlFont, sdlTexture', 'create_font', 'sdlRenderer renderer, FontData font_data',
    {
        cdef =
[[
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
]]
    })
}

local Gamepad = {
    c_code_block(
[[
/************************
 #                      #
 #       Gamepad        #
 #                      #
 ************************/
]]
    ),
    function_block('sdlGamepad', 'gamepad_open', 'integer index',
    {
        ccall =
[[
    SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (ctrl == NULL) lua_pushnil(L);
    else {
        NEW_UDATA(sdlGamepad, controller);
        *controller = ctrl;
    }
]]
    }),
    function_block('boolean', 'is_gamepad', 'integer joystick', { ccall = "PUSH_BOOLEAN(SDL_IsGameController(joystick));" }),
    function_block('integer', 'gamepad_get_axis_from_string', 'string name', { ccall = "PUSH_INTEGER(SDL_GameControllerGetAxisFromString(name));" }),
    function_block('integer', 'gamepad_get_button_from_string', 'string name', { ccall = "PUSH_INTEGER(SDL_GameControllerGetButtonFromString(name));" }),
}

local GLContext = {
    c_code_block(
[[
/************************
 #                      #
 #       GLContext      #
 #                      #
 ************************/
]]
    ),
    function_block('sdlGLContext', 'create_gl_context', 'sdlWindow win',
    {
        cdef =
[[
    INIT_ARG();
	CHECK_UDATA(sdlWindow, win);
	SDL_GLContext* glctx = SDL_GL_CreateContext(*win);
	if (glctx == NULL)
		return luaL_error(L, "[selene] failed to create GL Context: %s", SDL_GetError());
	NEW_UDATA(sdlGLContext, ctx);
	*ctx = glctx;
	return 1;
]]
    }),
    function_block('', 'gl_make_current', 'sdlWindow win, sdlGLContext ctx', {
        cdef =
[[
    INIT_ARG();
    CHECK_UDATA(sdlWindow, win);
    CHECK_UDATA(sdlGLContext, ctx);
    SDL_GL_MakeCurrent(*win, *ctx);
    return 0;     
]]
    }),
    function_block('lightuserdata', 'get_gl_proc_address_func', '', { ccall = "PUSH_LUDATA(SDL_GL_GetProcAddress);" }),
    function_block('lightuserdata', 'gl_get_proc_address', 'string name',
    {
        ccall =
[[
    void* proc = SDL_GL_GetProcAddress(name);
	PUSH_LUDATA(proc);
]]
    }),
    function_block('', 'gl_set_attribute', 'integer attr, integer value', { ccall = "SDL_GL_SetAttribute(attr, value);" }),
    function_block('boolean', 'gl_set_swap_interval', 'boolean value', { ccall = "PUSH_BOOLEAN(SDL_GL_SetSwapInterval(value) == 0);" }),
    function_block('integer', 'gl_get_swap_interval', '', { ccall = "PUSH_INTEGER(SDL_GL_GetSwapInterval());" }),
}

local Joystick = {
    c_code_block(
[[
/************************
 #                      #
 #       Joystick       #
 #                      #
 ************************/
]]
    ),
    function_block('sdlJoystick|nil', 'joystick_open', 'integer id',
    {
        ccall =
[[
    SDL_Joystick* j = SDL_JoystickOpen(id);
    if (j) {
        NEW_UDATA(sdlJoystick, joy);
        *joy = j;
    }
    else PUSH_NIL();
]]
    }),
    function_block('integer', 'num_joysticks', '', { ccall = "PUSH_INTEGER(SDL_NumJoysticks());" }),
}

--- @type generator.Group
local Renderer = {

    c_code_block(
[[
/************************
 #                      #
 #       Renderer       #
 #                      #
 ************************/
]]
    ),

    function_block('sdlRenderer', 'create_renderer', 'sdlWindow win, integer|nil index, integer|nil flags',
    {
        cdef =
[[
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
]]
    })
}

local RWops = {
    c_code_block(
[[
/************************
 #                      #
 #        RWops         #
 #                      #
 ************************/
]]
    ),
    function_block('lightuserdata, integer', 'load_file', 'string path',
    {
        ccall =
[[
    size_t size;
    void* data = SDL_LoadFile(path, &size);
    PUSH_LUDATA(data);
    PUSH_INTEGER(size);
]]
    }),
    function_block('sdlRWops', 'read_file', 'string path',
    {
        ccall =
[[
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
]]
    }),
    function_block('sdlRWops', 'rw_from_file', 'string path, string mode',
    {
        ccall =
[[
    SDL_RWops* ops = SDL_RWFromFile(path, mode);
    if (!ops) {
        lua_pushnil(L);
        return 1;
    }
    NEW_UDATA(sdlRWops, rwops);
    memcpy(rwops, ops, sizeof(SDL_RWops));
]]
    })
}

local Texture = {
    c_code_block (
[[
/************************
 #                      #
 #       Texture        #
 #                      #
 ************************/
]]
    ),
    function_block('sdlTexture', 'create_texture', 'sdlRenderer render, ImageData|integer data_or_format, integer|nil access, integer|nil width, integer|nil height',
    {
        cdef =
[[
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
]]
    })
}

local Window = {
    c_code_block (
[[
/************************
 #                      #
 #        Window        #
 #                      #
 ************************/
]]
    ),
    function_block('sdlWindow', 'create_window', 'string title, integer x, integer y, integer width, integer height, integer flags',
    {
        cdef =
[[
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
]]
    })
}


--- @type generator.Group
local Clipboard = {
    c_code_block (
[[
/************************
 #                      #
 #      Clipboard       #
 #                      #
 ************************/
]]
    ),

    function_block ('string', 'get_clipboard_text', '', {
        cdef =
[[
    char *text = SDL_GetClipboardText();
    PUSH_STRING(text);
    SDL_free(text);
    return 1;
]]
    }),
    function_block ('boolean', 'has_clipboard_text', '', { ccall = "PUSH_BOOLEAN(SDL_HasClipboardText());" }),
    function_block ('', 'set_clipboard_text', 'string text', { ccall = "SDL_SetClipboardText(luaL_checkstring(L, arg));" }),
}

--- @type generator.Group
local Filesystem = {
    c_code_block (
[[
/************************
 #                      #
 #      Filesystem      #
 #                      #
 ************************/
]]
    ),

    function_block ('string', 'get_base_path', '', {
        cdef =
[[
    char *path = SDL_GetBasePath();
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
]]
    }),
    function_block ('string', 'get_pref_path', 'string org, string app',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_STRING(org);
    CHECK_STRING(app);
    char *path = SDL_GetPrefPath(org, app);
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
]]
    }),
}

--- @type generator.Group
local Shared = {
    c_code_block (
[[
/************************
 #                      #
 #        Shared        #
 #                      #
 ************************/
]]
    ),

    function_block ('lightuserdata', 'load_object', 'string so_file', {
        cdef =
[[
    INIT_ARG();
    CHECK_STRING(sofile);
    void *obj = SDL_LoadObject(sofile);
    PUSH_LUDATA(obj);
    return 1;
]]
    }),
    function_block ('', 'unload_object', 'lightuserdata handle',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_LUDATA(void, handle);
    SDL_UnloadObject(handle);
    return 0;
]]
    }),
    function_block ('lightuserdata', 'load_function', 'lightuserdata handle, string name',
    {
        cdef =
[[
    INIT_ARG();
    GET_LUDATA(void, handle);
    CHECK_STRING(name);
    void *func = SDL_LoadFunction(handle, name);
    PUSH_LUDATA(func);
    return 1;
]]
    }),
}

--- @type generator.Group
local Keyboard = {
    c_code_block (
[[
/************************
 #                      #
 #       Keyboard       #
 #                      #
 ************************/
]]
    ),

    function_block ('lightuserdata', 'get_keyboard_state', '', {
        cdef =
[[
    void* data = (void*)SDL_GetKeyboardState(NULL);
    PUSH_LUDATA(data);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, data);
    return 1;
]]
    }),
    function_block ('', 'check_key', 'lightuserdata handle, integer key',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_LUDATA(Uint8, k);
    CHECK_INTEGER(i);
    lua_pushboolean(L, k[i]);
    return 1;
]]
    }),
    function_block ('boolean', 'had_screen_keyboard_support', '', { ccall = "PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());" }),
    function_block ('boolean', 'is_screen_keyboard_shown', 'sdlWindow win',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_UDATA(sdlWindow, win);
    PUSH_BOOLEAN(SDL_IsScreenKeyboardShown(*win));
    return 1;
]]
    }),
    function_block ('integer', 'get_scancode_from_name', 'string name', { ccall = "PUSH_INTEGER(SDL_GetScancodeFromName(name));" }),
    function_block ('string', 'get_scancode_name', 'integer scancode', { ccall = "PUSH_STRING(SDL_GetScancodeName(scancode));" }),
    function_block ('integer', 'get_key_from_name', 'string name', { ccall = "PUSH_INTEGER(SDL_GetKeyFromName(name));" }),
    function_block ('string', 'get_key_name', 'integer keycode', { ccall = "PUSH_STRING(SDL_GetKeyName(keycode));" }),
    function_block ('', 'start_text_input', '', { ccall = "SDL_StartTextInput();" }),
    function_block ('', 'stop_text_input', '', { ccall = "SDL_StopTextInput();" }),
}

--- @type generator.Group
local Mouse = {
    c_code_block (
[[
/************************
 #                      #
 #         Mouse        #
 #                      #
 ************************/
]]
    ),

    function_block ('number, number', 'get_mouse_position', '', {
        cdef =
[[
    int x, y;
    SDL_GetMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
]]
    }),
    function_block ('number, number', 'get_relative_mouse_position', '', {
        cdef =
[[
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
]]
    }),
    function_block('boolean', 'is_mouse_down', 'integer button', { ccall = "PUSH_BOOLEAN(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));" })
}

--- @type generator.Group
local Timer = {
    c_code_block (
[[
/************************
 #                      #
 #        Timer         #
 #                      #
 ************************/
]]
    ),

    function_block ('integer', 'get_ticks', '', { ccall = "PUSH_INTEGER(SDL_GetTicks());" }),
    function_block ('', 'delay', 'uinteger ms', { ccall = "SDL_Delay(ms);" }),
    function_block ('integer', 'get_performance_counter', '', { ccall = "PUSH_INTEGER(SDL_GetPerformanceCounter());" }),
    function_block ('integer', 'get_performance_frequency', '', { ccall = "PUSH_INTEGER(SDL_GetPerformanceFrequency());" }),
    function_block ('', 'set_timer', '', { ccall = "" }),
}


local lib = Lib.create('sdl')
lib:add_source(c_code_block(        
[[static SDL_Texture* texture_from_image_data(SDL_Renderer* renderer, int access, ImageData* img_data) {
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
]]
) )
lib:add_source(Core)
lib:add_source(Platforms)
lib:add_source(AudioDeviceID)
lib:add_source(AudioStream)
lib:add_source(Event)
lib:add_source(Font)
lib:add_source(Gamepad)
lib:add_source(GLContext)
lib:add_source(Joystick)
lib:add_source(Renderer)
lib:add_source(RWops)
lib:add_source(Texture)
lib:add_source(Window)
lib:add_source(Clipboard)
lib:add_source(Filesystem)
lib:add_source(Shared)
lib:add_source(Keyboard)
lib:add_source(Mouse)
lib:add_source(Timer)
lib.enums = require('enums')
lib.meta_types = require('meta_types')
lib.enum_prefix = 'SDL_'
lib.header_include =
[[
#include "platforms.h"
#include "common.h"
#include "selene.h"
#include "lua_helper.h"
]]

lib.header = {
    c_code_block(
[[

typedef struct {
    int top, count;
    sdlAudioStream* data;
    int* availables;
} AudioStreamPool;

typedef struct {
    FontData* font_data;
    sdlTexture* texture;
} sdlFont;
    
]]
    )
}

return lib