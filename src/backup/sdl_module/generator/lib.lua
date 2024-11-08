
--- @alias BlockType
--- | 'cdef'
--- | 'comment'
--- | 'module_function'
--- | 'breakline'
--- | 'emmy'

--- @alias Block {type:BlockType, value:string|nil}

--- @type Block[]
local blocks = {
    {
        type = 'cdef',
        value =
[[
static int l_hello_world(lua_State* L) {
    return 0;
}
]]
    },
    {
        type = 'comment',
        name = 'Core',
        value =
[[
/************************
 #                      #
 #         Core         #
 #                      #
 ************************/
]]
    },
    {
        type = 'module_function',
        name = 'init',
        args = {'integer flags'},
        returns = {'boolean'},
        ccall =
[[
    PUSH_BOOLEAN(SDL_Init(flags) == 0);
]]
    },
    {
        type = 'module_function',
        name = 'quit',
        args = {},
        returns = {},
        ccall =
[[
    SDL_Quit();
]]
    },
    {
        type = 'comment',
        value =
[[
/* Platforms */
]]
    },
    {
        type = 'module_function',
        name = 'is_android_tv',
        args = {},
        returns = {'boolean'},
        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 8)
    PUSH_BOOLEAN(SDL_IsAndroidTV() == 1);
#else
    PUSH_BOOLEAN(0);
#endif
]]
    },
    {
        type = 'module_function',
        name = 'is_chromebook',
        args = {},
        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsChromebook() == 1);
#else
    PUSH_BOOLEAN(0);
#endif
]]
    },
    {
        type = 'module_function',
        name = 'is_dex_mode',
        args = {},
        returns = {'boolean'},
        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsDeXMode() == 1);
#else
    PUSH_BOOLEAN(0);
#endif
]]
    },
    {
        type = 'module_function',
        name = 'is_tablet',
        args = {},
        returns = {'boolean'},
        ccall =
[[
    PUSH_BOOLEAN(SDL_IsTablet() == 1);
]]
    },
    {
        type = 'comment',
        value =
[[
/************************
 #                      #
 #     AudioDeviceID    #
 #                      #
 ************************/
]]
    },
    {
        name = 'open_audio_device',
        args = {
            {
                type = 'string',
                name = 'device'
            },
            {
                type = 'boolean',
                name = 'is_capture'
            },
            {
                type = 'table',
                name = 'specs'
            }
        },
        returns = {},
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
    return 1;
]]
    },
    {
        name = 'gen_num_audio_devices',
        args = {
            {
                type = 'boolean',
                name = 'is_capture'
            }
        },
        returns = {'integer'},
        ccall =
[[
    PUSH_INTEGER(SDL_GetNumAudioDevices(is_capture));
]]
    },
    {
        type = 'module_function',
        name = 'get_audio_device_name',
        args = {
            {
                type = 'integer',
                name = 'index'
            },
            {
                type = 'boolean',
                name = 'is_capture'
            }
        },
        returns = {'string'},
        ccall =
[[
    PUSH_STRING	(SDL_GetAudioDeviceName(index, is_capture));
]]
    },
    {
        type = 'comment',
        value =
[[
/************************
 #                      #
 #      AudioStream     #
 #                      #
 ************************/
]]
    },
    {
        name = 'new_audio_stream',
        args = {
            {
                type = 'table',
                name = 'in_spec'
            },
            {
                type = 'table',
                name = 'out_spec'
            },
        },
        returns = {'sdl.AudioStream'},
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
    },
    {
        type = 'comment',
        value =
[[
/************************
 #                      #
 #        Event         #
 #                      #
 ************************/
]]
    },
    {
        type = 'module_function',
        name = 'create_event',
        args = {},
        returns = {'sdlEvent'},
        ccall =
[[
NEW_UDATA(sdlEvent, ev);
]]
    },
    {
        type = 'comment',
        value =
[[
/************************
 #                      #
 #        Font          #
 #                      #
 ************************/
]]
    },
    {
        type = 'module_function',
        name = 'create_font',
        args = {
            {
                type = 'sdlRenderer',
                name = 'renderer'
            },
            {
                type = 'FontData',
                name = 'font_data'
            }
        },
        returns = {'sdlFont', 'sdlTexture'},
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
    },
    {
        type = 'comment',
        value =
[[
/************************
 #                      #
 #       Gamepad        #
 #                      #
 ************************/
]]
    },
    {
        type = 'module_function',
        name = 'gamepad_open',
        args = {
            {
                type = 'integer',
                name = 'index'
            }
        },
        returns = {'sdlGamepad'},
        ccall =
[[
    SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (ctrl == NULL) lua_pushnil(L);
    else {
        NEW_UDATA(sdlGamepad, controller);
        *controller = ctrl;
    }
]]
    },
    {
        type = 'module_function',
        name = 'is_gamepad',
        args = {
            {
                type = 'integer',
                name = 'joystick'
            }
        },
        returns = {'boolean'},
        ccall =
[[
    PUSH_BOOLEAN(SDL_IsGameController(joystick));
]]
    },
    {
        name = 'gamepad_get_axis_from_string',
        args = {
            {
                type = 'string',
                name = 'axis_name'
            }
        },
        ccall =
[[
INIT_ARG();
CHECK_STRING(str);
PUSH_INTEGER(SDL_GameControllerGetAxisFromString(str));
return 1;
]]
    },
    {
        name = 'gamepad_get_button_from_string',
        args = {
            {
                type = 'string',
                name = 'button_name'
            }
        },
        ccall =
[[
INIT_ARG();
CHECK_STRING(str);
PUSH_INTEGER(SDL_GameControllerGetButtonFromString(str));
return 1;
]]
    }
}

--- @class sdl.lib
--- @field name string
--- @field blocks Block[]
local lib = {
    name = 'sdl',
    blocks = blocks
}

return lib