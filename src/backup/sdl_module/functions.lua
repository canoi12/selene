return {
    lib = {
        blocks = {
            {
                name = "Core",
                description = [[
/************************
 #                      #
 #         Core         #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'init',
                        args = {
                            {
                                name = 'flags',
                                type = 'integer'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_NUMBER(unsigned int, flags);
    int err = SDL_Init(flags);
    PUSH_BOOLEAN(err == 0);
    return 1;
]]
                    },
                    {
                        name = 'quit',
                        args = {},
                        ccall =
[[
    SDL_Quit();
	return 0;
]]
                    }
                }
            },
            {
                name = "Platforms",
                functions = {
                    {
                        name = 'is_android_tv',
                        args = {},
                        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 8)
    PUSH_BOOLEAN(SDL_IsAndroidTV() == 1);
    return 1;
#else
    return 0;
#endif
]]
                    },
                    {
                        name = 'is_chromebook',
                        args = {},
                        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsChromebook() == 1);
    return 1;
#else
    return 0;
#endif
]]
                    },
                    {
                        name = 'is_dex_mode',
                        args = {},
                        ccall =
[[
#if defined(OS_ANDROID) && SDL_VERSION_ATLEAST(2, 0, 9)
    PUSH_BOOLEAN(SDL_IsDeXMode() == 1);
    return 1;
#else
    return 0;
#endif
]]
                    },
                    {
                        name = 'is_tablet',
                        args = {},
                        ccall =
[[
    PUSH_BOOLEAN(SDL_IsTablet() == 1);
    return 1;
]]
                    }
                }
            },
            {
                name = 'AudioDeviceID',
                description =
[[
/************************
 #                      #
 #     AudioDeviceID    #
 #                      #
 ************************/
]],
                functions = {
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
                        ccall =
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
                        ccall =
[[
    INIT_ARG();
	GET_BOOLEAN(is_capture);
	PUSH_INTEGER(SDL_GetNumAudioDevices(is_capture));
	return 1;
]]
                    },
                    {
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
                        ccall =
[[
    INIT_ARG();
	CHECK_INTEGER(index);
	GET_BOOLEAN(is_capture);
	PUSH_STRING	(SDL_GetAudioDeviceName(index, is_capture));
	return 1;
]]
                    }
                }
            },
            {
                name = 'AudioStream',
                description =
[[
/************************
 #                      #
 #      AudioStream     #
 #                      #
 ************************/
]],
                functions = {
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
                            }
                        },
                        ccall =
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
                    }
                }
            },
            {
                name = 'Event',
                description =
[[
/************************
 #                      #
 #        Event         #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'create_event',
                        args = {},
                        ccall =
[[
    NEW_UDATA(sdlEvent, ev);
    return 1;
]]
                    }
                }
            },
            {
                name = 'Font',
                description =
[[
/************************
 #                      #
 #        Font          #
 #                      #
 ************************/
]],
                functions = {
                    {
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
                        ccall =
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
                    }
                }
            },
            {
                name = 'Gamepad',
                description =
[[
/************************
 #                      #
 #       Gamepad        #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'gamepad_open',
                        args = {
                            {
                                type = 'integer',
                                name = 'index'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_INTEGER(index);
    SDL_GameController* ctrl = SDL_GameControllerOpen(index);
    if (ctrl == NULL) lua_pushnil(L);
    else {
        NEW_UDATA(sdlGamepad, controller);
        *controller = ctrl;
    }
    return 1;
]]
                    },
                    {
                        name = 'is_gamepad',
                        args = {
                            {
                                type = 'integer',
                                name = 'joystick'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_INTEGER(joy);
    PUSH_BOOLEAN(SDL_IsGameController(joy));
    return 1;
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
            },
            {
                name = 'GLContext',
                description =
[[
/************************
 #                      #
 #       GLContext      #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'create_gl_context',
                        args = {
                            {
                                type = 'sdlWindow',
                                name = 'win'
                            }
                        },
                        ccall =
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
                    },
                    {
                        name = 'gl_make_current',
                        args = {
                            {
                                type = 'sdlWindow',
                                name = 'win'
                            },
                            {
                                type = 'sdlGLContext',
                                name = 'ctx'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_UDATA(sdlWindow, win);
    CHECK_UDATA(sdlGLContext, ctx);
    SDL_GL_MakeCurrent(*win, *ctx);
    return 0;
]]
                    },
                    {
                        name = 'get_gl_proc_address_func',
                        args = {},
                        ccall =
[[
    PUSH_LUDATA(SDL_GL_GetProcAddress);
    return 1;
]]
                    },
                    {
                        name = 'gl_get_proc_address',
                        args = {
                            {
                                type = 'string',
                                name = 'proc_name'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
	CHECK_STRING(name);
	void* proc = SDL_GL_GetProcAddress(name);
	PUSH_LUDATA(proc);
	return 1;
]]
                    },
                    {
                        name = 'gl_set_attribute',
                        args = {
                            {
                                type = 'integer',
                                name = 'attr'
                            },
                            {
                                type = 'integer',
                                name = 'value'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_INTEGER(attr);
    CHECK_INTEGER(value);
    SDL_GL_SetAttribute(attr, value);
    return 0;
]]
                    },
                    {
                        name = 'gl_set_swap_interval',
                        args = {
                            {
                                type = 'boolean',
                                name = 'value'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    GET_BOOLEAN(value);
    int res = SDL_GL_SetSwapInterval(value);
    PUSH_BOOLEAN(res == 0);
    return 1;
]]
                    },
                    {
                        name = 'gl_get_swap_interval',
                        args = {},
                        ccall =
[[
    int res = SDL_GL_GetSwapInterval();
    PUSH_INTEGER(res);
    return 1;
]]
                    }
                }
            },
            {
                name = 'Joystick',
                description =
[[
/************************
 #                      #
 #       Joystick       #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'joystick_open',
                        args = {
                            {
                                type = 'integer',
                                name = 'id'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_INTEGER(id);
    SDL_Joystick* j = SDL_JoystickOpen(id);
    if (j) {
        NEW_UDATA(sdlJoystick, joy);
        *joy = j;
    }
    else PUSH_NIL();
    return 1;
]]
                    },
                    {
                        name = 'num_joysticks',
                        args = {},
                        ccall =
[[
    PUSH_INTEGER(SDL_NumJoysticks());
    return 1;
]]
                    }
                }
            },
            {
                name = 'Renderer',
                description =
[[
/************************
 #                      #
 #       Renderer       #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'create_renderer',
                        args = {
                            {
                                type = 'sdlWindow',
                                name = 'win'
                            },
                            {
                                type = 'integer|nil',
                                name = 'index'
                            },
                            {
                                type = 'integer',
                                name = 'flags'
                            }
                        },
                        ccall =
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
                    }
                }
            },
            {
                name = 'RWops',
                description =
[[
/************************
 #                      #
 #        RWops         #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'load_file',
                        args = {
                            {
                                type = 'string',
                                name = 'path'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_STRING(path);
    size_t size;
    void* data = SDL_LoadFile(path, &size);
    PUSH_LUDATA(data);
    PUSH_INTEGER(size);
    return 2;
]]
                    },
                    {
                        name = 'read_file',
                        args = {
                            {
                                type = 'string',
                                name = 'path'
                            }
                        },
                        ccall =
[[
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
]]
                    },
                    {
                        name = 'rw_from_file',
                        args = {
                            {
                                type = 'string',
                                name = 'path'
                            },
                            {
                                type = 'string',
                                name = 'mode'
                            }
                        },
                        ccall =
[[
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
]]
                    }
                }
            },
            {
                name = 'Texture',
                description =
[[
/************************
 #                      #
 #       Texture        #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'create_texture',
                        args = {
                            {
                                type = 'sdlRenderer',
                                name = 'render'
                            },
                            {
                                type = 'ImageData|integer',
                                name = 'data_or_format'
                            },
                            {
                                type = 'integer|nil',
                                name = 'access'
                            },
                            {
                                type = 'integer|nil',
                                name = 'width'
                            },
                            {
                                type = 'integer|nil',
                                name = 'height'
                            },
                        },
                        ccall =
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
                    }
                }
            },
            {
                name = 'Window',
                description =
[[
/************************
 #                      #
 #        Window        #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'create_window',
                        args = {
                            {
                                type = 'string',
                                name = 'title'
                            },
                            {
                                type = 'integer',
                                name = 'x'
                            },
                            {
                                type = 'integer',
                                name = 'y'
                            },
                            {
                                type = 'integer',
                                name = 'width'
                            },
                            {
                                type = 'integer',
                                name = 'height'
                            },
                            {
                                type = 'integer',
                                name = 'flags'
                            }
                        },
                        ccall =
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
                    }
                }
            },
            {
                name = 'Clipboard',
                description =
[[
/************************
 #                      #
 #      Clipboard       #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'get_clipboard_text',
                        args = {},
                        ccall =
[[
    char *text = SDL_GetClipboardText();
    PUSH_STRING(text);
    SDL_free(text);
    return 1;
]]
                    },
                    {
                        name = 'has_clipboard_text',
                        args = {},
                        ccall =
[[
    PUSH_BOOLEAN(SDL_HasClipboardText());
    return 1;
]]
                    },
                    {
                        name = 'set_clipboard_text',
                        args = {
                            {
                                name = 'text',
                                type = 'string'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_STRING(text);
    SDL_SetClipboardText(text);
    return 0;
]]
                    }
                }
            },
            {
                name = 'Filesystem',
                description =
[[
/************************
 #                      #
 #      Filesystem      #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'get_base_path',
                        args = {},
                        ccall =
[[
    char *path = SDL_GetBasePath();
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
]]
                    },
                    {
                        name = 'get_pref_path',
                        args = {},
                        ccall =
[[
    INIT_ARG();
    CHECK_STRING(org);
    CHECK_STRING(app);
    char *path = SDL_GetPrefPath(org, app);
    PUSH_STRING(path);
    SDL_free(path);
    return 1;
]]
                    }
                }
            },
            {
                name = 'Shared',
                description =
[[
/************************
 #                      #
 #        Shared        #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'load_object',
                        args = {
                            {
                                name = 'filename',
                                type = 'string'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_STRING(sofile);
    void *obj = SDL_LoadObject(sofile);
    PUSH_LUDATA(obj);
    return 1;
]]
                    },
                    {
                        name = 'unload_object',
                        args = {
                            {
                                type = 'lightuserdata',
                                name = 'handle'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_LUDATA(void, handle);
    SDL_UnloadObject(handle);
    return 0;
]]
                    },
                    {
                        name = 'load_function',
                        args = {
                            {
                                name = 'handle',
                                type = 'lightuserdata'
                            },
                            {
                                name = 'name',
                                type = 'string'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    GET_LUDATA(void, handle);
    CHECK_STRING(name);
    void *func = SDL_LoadFunction(handle, name);
    PUSH_LUDATA(func);
    return 1;
]]
                    }
                }
            },
            {
                name = 'Keyboard',
                description =
[[
/************************
 #                      #
 #       Keyboard       #
 #                      #
 ************************/
]],
                functions = {
--                     {
--                         name = 'check_key_state',
--                         args = {
--                             {
--                                 type = 'integer',
--                                 name = 'filename'
--                             }
--                         },
--                         ccall =
-- [[
--     INIT_ARG();
--     CHECK_INTEGER(key);
--     PUSH_BOOLEAN(keys[key]);
--     return 1;
-- ]]
--                     },
                    {
                        name = 'get_keyboard_state',
                        args = {},
                        ccall =
[[
    void* data = (void*)SDL_GetKeyboardState(NULL);
    PUSH_LUDATA(data);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, data);
    return 1;
]]
                    },
                    {
                        name = 'check_key',
                        args = {
                            {
                                type = 'lightuserdata',
                                name = 'keys'
                            },
                            {
                                type = 'integer',
                                name = 'index'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_LUDATA(Uint8, k);
    CHECK_INTEGER(i);
    lua_pushboolean(L, k[i]);
    return 1;
]]
                    },
                    {
                        name = 'has_screen_keyboard_support',
                        args = {},
                        ccall =
[[
    PUSH_BOOLEAN(SDL_HasScreenKeyboardSupport());
    return 1;
]]
                    },
                    {
                        name = 'is_screen_keyboard_shown',
                        args = {
                            {
                                type = 'sdlWindow',
                                name = 'win'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_UDATA(sdlWindow, win);
    PUSH_BOOLEAN(SDL_IsScreenKeyboardShown(*win));
    return 1;
]]
                    },
                    {
                        name = 'get_scancode_from_name',
                        args = {
                            {
                                type = 'string',
                                name = 'name'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GetScancodeFromName(name));
    return 1;
]]
                    },
                    {
                        name = 'get_scancode_name',
                        args = {
                            {
                                type = 'integer',
                                name = 'scancode'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_INTEGER(scancode);
    PUSH_STRING(SDL_GetScancodeName(scancode));
    return 1;
]]
                    },
                    {
                        name = 'get_key_from_name',
                        args = {
                            {
                                type = 'string',
                                name = 'name'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_STRING(name);
    PUSH_INTEGER(SDL_GetKeyFromName(name));
    return 1;
]]
                    },
                    {
                        name = 'get_key_name',
                        args = {
                            {
                                type = 'integer',
                                name = 'keycode'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_INTEGER(keycode);
    PUSH_STRING(SDL_GetKeyName(keycode));
    return 1;
]]
                    },
                    {
                        name = 'start_text_input',
                        args = {},
                        ccall =
[[
    SDL_StartTextInput();
    return 0;
]]
                    },
                    {
                        name = 'stop_text_input',
                        args = {},
                        ccall =
[[
    SDL_StopTextInput();
    return 0;
]]
                    }
                }
            },
            {
                name = 'Mouse',
                description =
[[
/************************
 #                      #
 #         Mouse        #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'get_mouse_position',
                        args = {},
                        returns = {'integer', 'integer'},
                        ccall =
[[
    int x, y;
    SDL_GetMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
]]
                    },
                    {
                        name = 'get_relative_mouse_position',
                        args = {},
                        returns = {'integer', 'integer'},
                        ccall =
[[
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    PUSH_NUMBER(x);
    PUSH_NUMBER(y);
    return 2;
]]
                    },
                    {
                        name = 'is_mouse_down',
                        args = {
                            {
                                type = 'integer',
                                name = 'button'
                            }
                        },
                        returns = {'boolean'},
                        ccall =
[[
    INIT_ARG();
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button));
    return 1;
]]
                    }
                }
            },
            {
                name = 'Timer',
                description =
[[
/************************
 #                      #
 #        Timer         #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'get_ticks',
                        args = {},
                        returns = {'integer'},
                        ccall =
[[
    PUSH_INTEGER(SDL_GetTicks());
    return 1;
]]
                    },
                    {
                        name = 'delay',
                        args = {
                            {
                                type = 'integer',
                                name = 'ms'
                            }
                        },
                        ccall =
[[
    Uint32 ms = (Uint32)luaL_checknumber(L, 1);
    SDL_Delay(ms);
    return 0;
]]
                    },
                    {
                        name = 'get_performance_counter',
                        args = {},
                        returns = {'integer'},
                        ccall =
[[
    PUSH_INTEGER(SDL_GetPerformanceCounter());
    return 1;
]]
                    },
                    {
                        name = 'get_performance_frequency',
                        args = {},
                        returns = {'integer'},
                        ccall =
[[
    PUSH_INTEGER(SDL_GetPerformanceFrequency());
    return 1;
]]
                    }
                }
            },
            {
                name = 'Video',
                description =
[[
/************************
 #                      #
 #        Video         #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'enable_screen_saver',
                        args = {},
                        ccall =
[[
    SDL_EnableScreenSaver();
    return 0;
]]
                    },
                }
            },
            {
                name = 'Error',
                description =
[[
/************************
 #                      #
 #        Error         #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'get_error',
                        args = {},
                        returns = {'string'},
                        ccall =
[[
    PUSH_STRING(SDL_GetError());
    return 1;
]]
                    },
                    {
                        name = 'set_error',
                        args = {
                            {
                                type = 'string',
                                name = 'msg'
                            }
                        },
                        ccall =
[[
    INIT_ARG();
    CHECK_STRING(msg);
    #if !defined(OS_ANDROID)
    SDL_SetError("%s", msg);
    #endif
    return 0;
]]
                    }
                }
            },
            {
                name = 'System',
                description =
[[
/************************
 #                      #
 #        System        #
 #                      #
 ************************/
]],
                functions = {
                    {
                        name = 'get_platform',
                        args = {},
                        returns = {'string'},
                        ccall =
[[
    PUSH_STRING(SDL_GetPlatform());
    return 1;
]]
                    },
                    {
                        name = 'get_cpu_cache_line_size',
                        args = {},
                        returns = {'integer'},
                        ccall =
[[
    PUSH_INTEGER(SDL_GetCPUCacheLineSize());
    return 1;
]]
                    },
                    {
                        name = 'get_cpu_count',
                        args = {},
                        returns = {'integer'},
                        ccall =
[[
    PUSH_INTEGER(SDL_GetCPUCount());
    return 1;
]]
                    },
                    {
                        name = 'get_system_ram',
                        args = {},
                        returns = {'integer'},
                        ccall =
[[
    PUSH_INTEGER(SDL_GetSystemRAM());
    return 1;
]]
                    }
                }
            },
        }
    },
    metas = {
        {
            name = 'AudioDeviceID',
            functions = {
                {
                    name = 'pause',
                    args = {
                        {
                            type = 'boolean',
                            name = 'pause'
                        }
                    },
                    ccall =
[[
    CHECK_META(sdlAudioDeviceID);
    GET_BOOLEAN(pause);
    SDL_PauseAudioDevice(*self, pause);
    return 0;
]]
                },
                {
                    name = 'close',
                    args = {},
                    ccall =

[[
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
]]
                }
            }
        },
        {
            name = 'AudioStream',
            functions = {
                {
                    name = 'clear',
                    args = {},
                    ccall =
[[
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamClear(*self);
    return 0;
]]
                },
                {
                    name = 'free',
                    args = {},
                    ccall =
[[
    CHECK_META(sdlAudioStream);
    SDL_FreeAudioStream(*self);
    return 0;
]]
                },
                {
                    name = 'flush',
                    args = {},
                    ccall =
[[
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamFlush(*self);
    return 0;
]]
                },
                {
                    name = 'bind',
                    args = {
                        {
                            type = 'AudioDeviceID',
                            name = 'dev'
                        }
                    },
                    ccall =
[[
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(sdlAudioDeviceID, dev);
    PUSH_VALUE(-1);
    lua_rawget(L, LUA_REGISTRYINDEX);
    AudioStreamPool* pool = lua_touserdata(L, -1);
    if (pool->top <= 0)
        return luaL_error(L, "AudioStreamPool is full\n");
    int index = pool->availables[--(pool->top)];
    pool->data[index] = *self;
    return 0;
]]
                },
                {
                    name = 'unbind',
                    args = {},
                    ccall =
[[
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(sdlAudioDeviceID, dev);
    PUSH_VALUE(-1);
    lua_rawget(L, LUA_REGISTRYINDEX);
    AudioStreamPool* pool = lua_touserdata(L, -1);
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
]]
                },
                {
                    name = 'put',
                    args = {},
                    ccall =
[[
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
            data = d->root;
            len = d->size;
        }
        break;
    }
    int res = SDL_AudioStreamPut(*self, data, (int)len);
    PUSH_INTEGER(res);
    return 1;
]]
                },
                {
                    name = 'get',
                    args = {
                        {
                            type = 'Data',
                            name = 'out'
                        }
                    },
                    returns = {'integer'},
                    ccall =
[[
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(Data, out);
    OPT_INTEGER(size, out->size);
    int res = SDL_AudioStreamGet(*self, out->root, size);
    PUSH_INTEGER(res);
    return 1;
]]
                },
                {
                    name = 'available',
                    args = {},
                    ccall =
[[
    CHECK_META(sdlAudioStream);
    int res = SDL_AudioStreamAvailable(*self);
    PUSH_INTEGER(res);
    return 1;
]]
                },
            }
        },
        {
            name = 'Event',
            functions = {
                {
                    name = "poll",
                    args = {},
                    returns = {'boolean'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_BOOLEAN(SDL_PollEvent(self));
    return 1;
]]
                },
                {
                    name = "get_type",
                    args = {},
                    returns = {'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->type);
    return 1;
]]
                },
                {
                    name = "window_event",
                    args = {},
                    returns = {'integer', 'integer', 'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->window.event);
    PUSH_INTEGER(self->window.windowID);
    PUSH_INTEGER(self->window.data1);
    PUSH_INTEGER(self->window.data2);
    return 4;
]]
                },
                {
                    name = "audio_device_event",
                    args = {},
                    returns = {'integer', 'integer', 'integer', 'integer', 'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->adevice.which);
    return 1;
]]
                },
                {
                    name = "mouse_motion_event",
                    args = {},
                    returns = {'integer', 'integer', 'integer', 'integer', 'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->motion.which);
    PUSH_INTEGER(self->motion.x);
    PUSH_INTEGER(self->motion.y);
    PUSH_INTEGER(self->motion.xrel);
    PUSH_INTEGER(self->motion.yrel);
    PUSH_INTEGER(self->motion.state);
    return 6;
]]
                },
                {
                    name = "mouse_button_event",
                    args = {},
                    returns = {'integer', 'integer', 'integer', 'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->button.which);
    PUSH_INTEGER(self->button.button);
    PUSH_INTEGER(self->button.x);
    PUSH_INTEGER(self->button.y);
    PUSH_INTEGER(self->button.clicks);
    return 5;
]]
                },
                {
                    name = "mouse_wheel_event",
                    args = {},
                    returns = {'integer', 'integer', 'integer', 'boolean'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->wheel.which);
    PUSH_INTEGER(self->wheel.x);
    PUSH_INTEGER(self->wheel.y);
    PUSH_BOOLEAN(self->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
    return 4;
]]
                },
                {
                    name = "keyboard_event",
                    args = {},
                    returns = {'integer', 'integer', 'boolean', 'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->key.keysym.scancode);
    PUSH_INTEGER(self->key.state == SDL_PRESSED);
    PUSH_BOOLEAN(self->key.repeat);
    PUSH_INTEGER(self->key.keysym.sym);
    PUSH_INTEGER(self->key.keysym.mod);
    return 5;
]]
                },
                {
                    name = "joy_hat_event",
                    args = {},
                    returns = {'integer', 'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jhat.which);
    PUSH_INTEGER(self->jhat.hat);
    PUSH_INTEGER(self->jhat.value);
    return 3;
]]
                },
                {
                    name = "text_event",
                    args = {},
                    returns = {'string'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_STRING(self->text.text);
    return 1;
]]
                },
                {
                    name = "text_edit_event",
                    args = {},
                    returns = {'string', 'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_STRING(self->edit.text);
    PUSH_INTEGER(self->edit.start);
    PUSH_INTEGER(self->edit.length);
    return 3;
]]
                },
                {
                    name = "gamepad_device_event",
                    args = {},
                    returns = {'integer'},
                    ccall =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->cdevice.which);
    return 1;
]]
                },
            }
        },
        {
            name = 'Gamepad',
            functions = {
                { 
                    name = "get_name",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    PUSH_STRING(SDL_GameControllerName(*self));
    return 1;
]]
                },
                { 
                    name = "get_vendor",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetVendor(*self));
    return 1;
]]
                },
                { 
                    name = "get_product",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProduct(*self));
    return 1;
]]
                },
                { 
                    name = "get_product_version",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*self));
    return 1;
]]
                },
                { 
                    name = "get_axis",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*self, axis));
    return 1;
]]
                },
                { 
                    name = "get_button",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GameControllerGetButton(*self, button) == 1);
    return 1;
]]
                },
                { 
                    name = "rumble",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    OPT_INTEGER(freq, 100);
    PUSH_NUMBER(SDL_GameControllerRumble(*self, low, high, freq));
    return 1;
]]
                },
                { 
                    name = "current_power_level",
                    args = {},
                    returns = {},
                    ccall =
[[
    const char *gpad_powerlevels[] = {
        "unknown", "empty", "low", "medium", "high", "full", "wired"
    };
    CHECK_META(sdlGamepad);
    SDL_Joystick* j = SDL_GameControllerGetJoystick(*self);
    int level = SDL_JoystickCurrentPowerLevel(j)+1;
    PUSH_STRING(gpad_powerlevels[level]);
    return 1;
]]
                },
                { 
                    name = "close",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGamepad);
    if (SDL_GameControllerGetAttached(*self))
        SDL_GameControllerClose(*self);
    return 0;
]]
                },
            }
        },
        {
            name = 'GLContext',
            functions = {
                {
                    name = 'destroy',
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlGLContext);
    SDL_GL_DeleteContext(*self);
    return 0;
]]
                }
            }
        },
        {
            name = 'Joystick',
            functions = {
                {
                    name = "get_name",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_STRING(SDL_JoystickName(*self));
    return 1;
]]
                },
                {
                    name = "get_vendor",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetVendor(*self));
    return 1;
]]
                },
                {
                    name = "get_product",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProduct(*self));
    return 1;
]]
                },
                {
                    name = "get_product_version",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*self));
    return 1;
]]
                },
                {
                    name = "get_type",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetType(*self));
    return 1;
]]
                },
                {
                    name = "get_type_string",
                    args = {},
                    returns = {},
                    ccall =
[[
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
    CHECK_META(sdlJoystick);
    PUSH_STRING(joy_types[SDL_JoystickGetType(*self)]);
    return 1;
]]
                },
                {
                    name = "num_axes",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumAxes(*self));
    return 1;
]]
                },
                {
                    name = "num_balls",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumBalls(*self));
    return 1;
]]
                },
                {
                    name = "num_hats",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumHats(*self));
    return 1;
]]
                },
                {
                    name = "num_buttons",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumButtons(*self));
    return 1;
]]
                },
                {
                    name = "get_axis",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*self, axis));
    return 1;
]]
                },
            {
                    name = "get_ball",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*self, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
    return 2;
]]
                },
                {
                    name = "get_hat",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*self, hat));
    return 1;
]]
                },
                {
                    name = "get_button",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_JoystickGetButton(*self, button));
    return 1;
]]
                },
                {
                    name = "rumble",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*self, low, high, freq) == 0);
    return 1;
]]
                },
                {
                    name = "current_power_level",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*self));
    return 1;
]]
                },
                {
                    name = "close",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlJoystick);
    if (SDL_JoystickGetAttached(*self))
        SDL_JoystickClose(*self);
    return 0;
]]
                },
            }
        },
        {
            name = 'Renderer',
            functions = {
                {
                    name = "destroy",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    SDL_DestroyRenderer(*self);
    return 0;
]]
                },
                {
                    name = "clear",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    SDL_RenderClear(*self);
    return 0;
]]
                },
                {
                    name = "set_color",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    Uint8 d[4];
    for (int i = 0; i < 4; i++) d[i] = (Uint8)luaL_checkinteger(L, i+2);
    SDL_SetRenderDrawColor(*self, d[0], d[1], d[2], d[3]);
    return 0;
]]
                },
                {
                    name = "set_target",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    const TEST_UDATA(sdlTexture, tex);
    SDL_SetRenderTarget(*self, *tex);
    return 0;
]]
                },
                {
                    name = "set_clip_rect",
                    args = {},
                    returns = {},
                    ccall =
[[
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
]]
                },
                {
                    name = "set_blend_mode",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    CHECK_INTEGER(blendMode);
    SDL_SetRenderDrawBlendMode(*self, blendMode);
    return 0;
]]
                },
                {
                    name = "draw_point",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, x);
    const CHECK_NUMBER(float, y);
    SDL_RenderDrawPointF(*self, x, y);
    return 0;
]]
                },
                {
                    name = "draw_line",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, x0);
    const CHECK_NUMBER(float, y0);
    const CHECK_NUMBER(float, x1);
    const CHECK_NUMBER(float, y1);
    SDL_RenderDrawLineF(*self, x0, y0, x1, y1);
    return 0;
]]
                },
                {
                    name = "draw_triangle",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    float pos[6];
    for (int i = 0; i < 6; i++) pos[i] = (float)luaL_checknumber(L, i+2);
    SDL_RenderDrawLineF(*self, pos[0], pos[1], pos[2], pos[3]);
    SDL_RenderDrawLineF(*self, pos[2], pos[3], pos[4], pos[5]);
    SDL_RenderDrawLineF(*self, pos[4], pos[5], pos[0], pos[1]);
    return 0;
]]
                },
                {
                    name = "fill_triangle",
                    args = {},
                    returns = {},
                    ccall =
[[
    return 0;
]]
                },
                {
                    name = "draw_rect",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    float d[4];
    for (int i = 0; i < 4; i++) {
        d[i] = (float)luaL_checknumber(L, i+2);
    }
    SDL_FRect rect = {d[0], d[1], d[2], d[3]};
    SDL_RenderDrawRectF(*self, &rect);
    return 0;
]]
                },
                {
                    name = "fill_rect",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    float d[4];
    for (int i = 0; i < 4; i++) {
        d[i] = (float)luaL_checknumber(L, i+2);
    }
    SDL_FRect rect = {d[0], d[1], d[2], d[3]};
    SDL_RenderFillRectF(*self, &rect);
    return 0;
]]
                },
                {
                    name = "draw_circle",
                    args = {},
                    returns = {},
                    ccall =
[[
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
]]
                },
                {
                    name = "fill_circle",
                    args = {},
                    returns = {},
                    ccall =
[[
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
]]
                },
                {
                    name = "copy",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    CHECK_UDATA(sdlTexture, tex);
    SDL_Rect* psrc = NULL;
    SDL_FRect* pdst = NULL;
    if (lua_type(L, arg) == LUA_TTABLE) {
        int src[4];
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg, i+1);
            src[i] = (int)luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        psrc = (SDL_Rect*)src;
    }
    if (lua_type(L, arg+1) == LUA_TTABLE) {
        float dst[4];
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg+1, i+1);
            dst[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        pdst = (SDL_FRect*)dst;
    }
    
    SDL_RenderCopyF(*self, *tex, psrc, pdst);
    return 0;
]]
                },
                {
                    name = "copy_ex",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    CHECK_UDATA(sdlTexture, tex);
    SDL_Rect* psrc = NULL;
    SDL_FRect* pdst = NULL;
    if (lua_type(L, arg) == LUA_TTABLE) {
        int src[4];
        for (int i = 0; i < 4; i++) {
            lua_rawgeti(L, arg, i+1);
            src[i] = (int)luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        psrc = (SDL_Rect*)src;
    }
    if (lua_type(L, arg+1) == LUA_TTABLE) {
        float dst[4];
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
    if (lua_type(L, arg) == LUA_TTABLE) {
        float ctr[2];
        for (int i = 0; i < 2; i++) {
            lua_rawgeti(L, arg+1, i+1);
            ctr[i] = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        center = (SDL_FPoint*)ctr;
    }
    arg += 1;
    OPT_INTEGER(flip, SDL_FLIP_NONE);
    SDL_RenderCopyExF(*self, *tex, psrc, pdst, angle, center, flip);
    return 0;
]]
                },
                {
                    name = "print",
                    args = {},
                    returns = {},
                    ccall =
[[
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
    int w = font->font_data->image_width;
    int h = font->font_data->image_height;
    while (*p != 0) {
        int codepoint;
        p = utf8_codepoint(p, &codepoint);
        SDL_FRect dest;
        int pos[2];
        int src[4];
        char_rect(font->font_data, codepoint, &x0, &y0, pos, src, 0);
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
        SDL_RenderCopyF(*self, *font->texture, &t, &dest);
    }
    // fprintf(stderr, "\n");
    
    return 0;
]]
                },
                {
                    name = "flush",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    SDL_RenderFlush(*self);
    return 0;
]]
                },
                {
                    name = "present",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlRenderer);
    SDL_RenderPresent(*self);
    return 0;
]]
                },
            }
        },
        {
            name = 'RWops',
            functions = {
                {
                    name = 'read',
                    args = {
                        {
                            type = 'lightuserdata',
                            name = 'data'
                        },
                        {
                            type = 'integer',
                            name = 'size'
                        },
                        {
                            type = 'integer|nil',
                            name = 'maxnum'
                        }
                    },
                    returns = {'integer'},
                    ccall =
[[
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(maxnum, 1);
    size_t read = SDL_RWread(self, data, size, maxnum);
    PUSH_INTEGER(read);
    return 1;
]]
                },
                {
                    name = 'write',
                    args = {
                        {
                            type = 'lightuserdata',
                            name = 'data'
                        },
                        {
                            type = 'integer',
                            name = 'size'
                        },
                        {
                            type = 'integer|nil',
                            name = 'num'
                        }
                    },
                    returns = {'integer'},
                    ccall =
[[
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(num, 1);
    size_t written = SDL_RWwrite(self, data, size, num);
    PUSH_INTEGER(written);
    return 1;
]]
                },
                {
                    name = 'seek',
                    ccall =
[[
    CHECK_META(sdlRWops);
    CHECK_INTEGER(offset);
    OPT_INTEGER(whence, RW_SEEK_SET);
    Sint64 res = SDL_RWseek(self, offset, whence);
    PUSH_INTEGER(res);
    return 1;
]]
                },
                {
                    name = 'size',
                    ccall =
[[
    CHECK_META(sdlRWops);
    Sint64 size = SDL_RWsize(self);
    PUSH_INTEGER(size);
    return 1;
]]
                },
                {
                    name = 'tell',
                    ccall =
[[
    CHECK_META(sdlRWops);
    Sint64 offset = SDL_RWtell(self);
    PUSH_INTEGER(offset);
    return 1;
]]
                },
                {
                    name = 'close',
                    ccall =
[[
    CHECK_META(sdlRWops);
    Sint64 res = SDL_RWclose(self);
    PUSH_BOOLEAN(res == 0);
    return 1;
]]
                }
            }
        },
        {
            name = 'Texture',
            functions = {
                {
                    name = "destroy",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlTexture);
    SDL_DestroyTexture(*self);
    return 0;
]]
                },
                {
                    name = "query",
                    args = {},
                    returns = {'integer', 'integer', 'integer', 'integer'},
                    ccall =
[[
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
]]
                },
                {
                    name = "set_alpha_mod",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(alpha);
    SDL_SetTextureAlphaMod(*self, (Uint8)alpha);
    return 0;
]]
                },
                {
                    name = "set_color_mod",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(r);
    CHECK_INTEGER(g);
    CHECK_INTEGER(b);
    SDL_SetTextureColorMod(*self, r, g, b);
    return 0;
]]
                },
                {
                    name = "set_blend_mode",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(blendMode);
    SDL_SetTextureBlendMode(*self, blendMode);
    return 0;
]]
                },
                {
                name = "set_scale_mode",
                    args = {
                        {
                            type = 'integer',
                            name = 'scale_mode'
                        }
                    },
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(scaleMode);
    SDL_SetTextureScaleMode(*self, scaleMode);
    return 0;
]]
                },
            }
        },
        {
            name = 'Window',
            functions = {
                {
                    name = "destroy",
                    args = {},
                    ccall =
[[
    CHECK_META(sdlWindow);
    SDL_DestroyWindow(*self);
    return 0;
]]
                },
                {
                    name = "get_size",
                    args = {},
                    returns = {'integer', 'integer'},
                    ccall =
[[
    INIT_ARG();
    CHECK_UDATA(sdlWindow, window);
    int width, height;
    SDL_GetWindowSize(*window, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
    return 2;
]]
                },
                {
                    name = "set_size",
                    args = {},
                    returns = {},
                    ccall =
[[
    return 0;
]]
                },
                {
                    name = "get_position",
                    args = {},
                    returns = {'integer', 'integer'},
                    ccall =
[[
    CHECK_META(sdlWindow);
    int x, y;
    SDL_GetWindowPosition(*self, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
]]
                },
                {
                    name = "set_position",
                    args = {},
                    returns = {},
                    ccall =
[[
]]
                },
                {
                    name = "gl_swap",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlWindow);
    SDL_GL_SwapWindow(*self);
    return 0;
]]
                },
                {
                    name = "set_bordered",
                    args = {},
                    returns = {},
                    ccall =
[[
]]
                },
                {
                    name = "maximize",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlWindow);
    SDL_MaximizeWindow(*self);
    return 0;
]]
                },
                {
                    name = "minimize",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlWindow);
    SDL_MinimizeWindow(*self);
    return 0;
]]
                },
                {
                    name = "restore",
                    args = {},
                    returns = {},
                    ccall =
[[
    CHECK_META(sdlWindow);
    SDL_RestoreWindow(*self);
    return 0;
]]
                },
                {
                    name = "show_simple_message_box",
                    args = {},
                    returns = {},
                    ccall =
[[
]]
                },
            }
        },
    }
}