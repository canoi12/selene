--- @type generator.MetaLib[]
local meta_types = {}

--- SDL_AudioDeviceID
meta_types[1] = {
    name = 'sdlAudioDeviceID',
    typename = 'sdl.AudioDeviceID',
    ctype  = 'SDL_AudioDeviceID',
    blocks = {
    function_block('', 'pause', 'sdlAudioDeviceID self, boolean pause',
    {
        cdef =
[[
    CHECK_META(sdlAudioDeviceID);
    GET_BOOLEAN(pause);
    SDL_PauseAudioDevice(*self, pause);
    return 0;
]]
    }),
    function_block('', 'close', 'sdlAudioDeviceID self',
    {
        cdef =
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
    }),
    }
}

--- SDL_AudioStream
meta_types[2] = {
    name = 'sdlAudioStream',
    typename = 'sdl.AudioStream',
    ctype  = 'SDL_AudioStream*',
    blocks = {
    function_block('', 'clear', 'sdlAudioStream self',
    {
        cdef =
[[
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamClear(*self);
    return 0;
]]
    }),
    function_block('', 'free', 'sdlAudioStream self',
    {
        cdef =
[[
    CHECK_META(sdlAudioStream);
    SDL_FreeAudioStream(*self);
    return 0;
]]
    }),
    function_block('', 'flush', 'sdlAudioStream self',
    {
        cdef =
[[
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamFlush(*self);
    return 0;
]]
    }),
    function_block('', 'bind', 'sdlAudioStream self, sdlAudioDeviceID dev',
    {
        cdef =
[[
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
]]
    }),
    function_block('', 'unbind', 'sdlAudioStream self, sdlAudioDeviceID dev',
    {
        cdef =
[[
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
]]
    }),
    function_block('integer', 'put', 'sdlAudioStream self',
    {
        cdef =
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
    }),
    function_block('integer', 'get', 'sdlAudioStream self, Data out',
    {
        cdef =
[[
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(Data, out);
    OPT_INTEGER(size, out->size);
    int res = SDL_AudioStreamGet(*self, out->root, size);
    PUSH_INTEGER(res);
    return 1;
]]
    }),
    function_block('integer', 'available', 'sdlAudioStream self',
    {
        cdef =
[[
    CHECK_META(sdlAudioStream);
    int res = SDL_AudioStreamAvailable(*self);
    PUSH_INTEGER(res);
    return 1;
]]
    })
    }
}

--- SDL_Event
meta_types[3] = {
    name = 'sdlEvent',
    typename = 'sdl.Event',
    ctype  = 'SDL_Event',
    blocks = {
    function_block('boolean', 'poll', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_BOOLEAN(SDL_PollEvent(self));
    return 1;
]]
    }),
    function_block('integer', 'get_type', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->type);
    return 1;
]]
    }),
    function_block('integer, integer, integer, integer', 'window_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->window.event);
    PUSH_INTEGER(self->window.windowID);
    PUSH_INTEGER(self->window.data1);
    PUSH_INTEGER(self->window.data2);
    return 4;
]]
    }),
    function_block('integer, integer, integer, integer, integer, integer', 'mouse_motion_event', 'sdlEvent self',
    {
        cdef =
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
    }),
    function_block('integer, integer, integer, integer, integer', 'mouse_button_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->button.which);
    PUSH_INTEGER(self->button.button);
    PUSH_INTEGER(self->button.x);
    PUSH_INTEGER(self->button.y);
    PUSH_INTEGER(self->button.clicks);
    return 5;
]]
    }),
    function_block('integer, integer, integer, integer', 'mouse_wheel_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->wheel.which);
    PUSH_INTEGER(self->wheel.x);
    PUSH_INTEGER(self->wheel.y);
    PUSH_BOOLEAN(self->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
    return 4;
]]
    }),
    function_block('integer, integer, boolean, integer, integer', 'keyboard_event', 'sdlEvent self, Data out',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->key.keysym.scancode);
    PUSH_INTEGER(self->key.state == SDL_PRESSED);
    PUSH_BOOLEAN(self->key.repeat);
    PUSH_INTEGER(self->key.keysym.sym);
    PUSH_INTEGER(self->key.keysym.mod);
    return 5;
]]
    }),
    function_block('integer', 'audio_device_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->adevice.which);
    return 1;
]]
    }),
    function_block('integer', 'gamepad_device_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->cdevice.which);
    return 1;
]]
    }),
    function_block('integer, integer, integer', 'gamepad_axis_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->caxis.which);
    PUSH_INTEGER(self->caxis.axis);
    PUSH_INTEGER(self->caxis.value);
    return 3;
]]
    }),
    function_block('integer, integer, integer', 'gamepad_button_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->cbutton.which);
    PUSH_INTEGER(self->cbutton.button);
    PUSH_INTEGER(self->cbutton.state);
    return 3;
]]
    }),
    function_block('integer', 'joy_device_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jdevice.which);
    return 1;
]]
    }),
    function_block('integer, integer, integer', 'joy_axis_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jaxis.which);
    PUSH_INTEGER(self->jaxis.axis);
    PUSH_INTEGER(self->jaxis.value);
    return 3;
]]
    }),
    function_block('integer, integer, integer, integer', 'joy_ball_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jball.which);
    PUSH_INTEGER(self->jball.ball);
    PUSH_INTEGER(self->jball.xrel);
    PUSH_INTEGER(self->jball.yrel);
    return 4;
]]
    }),
    function_block('integer, integer, integer', 'joy_button_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jbutton.which);
    PUSH_INTEGER(self->jbutton.button);
    PUSH_INTEGER(self->jbutton.state);
    return 3;
]]
    }),
    function_block('integer, integer, integer', 'joy_hat_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->jhat.which);
    PUSH_INTEGER(self->jhat.hat);
    PUSH_INTEGER(self->jhat.value);
    return 3;
]]
    }),
    function_block('integer, integer, integer, integer, integer, integer', 'dollar_gesture_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_INTEGER(self->dgesture.touchId);
    PUSH_INTEGER(self->dgesture.gestureId);
    PUSH_INTEGER(self->dgesture.numFingers);
    PUSH_NUMBER(self->dgesture.error);
    PUSH_NUMBER(self->dgesture.x);
    PUSH_NUMBER(self->dgesture.y);
    return 6;
]]
    }),
    function_block('integer', 'text_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_STRING(self->text.text);
    return 1;
]]
    }),
    function_block('integer, integer, integer', 'text_edit_event', 'sdlEvent self',
    {
        cdef =
[[
    CHECK_META(sdlEvent);
    PUSH_STRING(self->edit.text);
    PUSH_INTEGER(self->edit.start);
    PUSH_INTEGER(self->edit.length);
    return 3;
]]
    }),
    }
}

--- SDL_Gamepad
meta_types[4] = {
    name = 'sdlGamepad',
    typename = 'sdl.Gamepad',
    ctype  = 'SDL_GameController*',
    blocks = {
    function_block('string', 'get_name', 'sdlGamepad self',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    PUSH_STRING(SDL_GameControllerName(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_vendor', 'sdlGamepad self',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetVendor(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_product', 'sdlGamepad self',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProduct(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_product_version', 'sdlGamepad self',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    PUSH_INTEGER(SDL_GameControllerGetProductVersion(*self));
    return 1;
]]
    }),
    c_code_block(
[[
const char *gpad_axes[] = {
    "leftx", "lefty", "rightx", "righty", "triggerleft", "triggerright"
};
]]
    ),
    function_block('number', 'get_axis', 'sdlGamepad self, integer axis',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(axis);
    PUSH_NUMBER(SDL_GameControllerGetAxis(*self, axis));
    return 1;
]]
    }),
    function_block('boolean', 'get_button', 'sdlGamepad self, integer button',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_GameControllerGetButton(*self, button) == 1);
    return 1;
]]
    }),
    function_block('number', 'rumble', 'sdlGamepad self, integer low, integer high, integer|nil freq',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    OPT_INTEGER(freq, 100);
    PUSH_NUMBER(SDL_GameControllerRumble(*self, low, high, freq));
    return 1;
]]
    }),
    c_code_block(
[[
const char *gpad_powerlevels[] = {
    "unknown", "empty", "low", "medium", "high", "full", "wired"
};
]]
    ),
    function_block('string', 'current_power_level', 'sdlGamepad self',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    SDL_Joystick* j = SDL_GameControllerGetJoystick(*self);
    int level = SDL_JoystickCurrentPowerLevel(j)+1;
    PUSH_STRING(gpad_powerlevels[level]);
    return 1;
]]
    }),
    function_block('', 'close', 'sdlGamepad self',
    {
        cdef =
[[
    CHECK_META(sdlGamepad);
    if (SDL_GameControllerGetAttached(*self))
        SDL_GameControllerClose(*self);
    return 0;
]]
    }),
    }
}

--- SDL_GLContext
meta_types[5] = {
    name = 'sdlGLContext',
    typename = 'sdl.GLContext',
    ctype  = 'SDL_GLContext*',
    blocks = {
    function_block('', 'destroy', 'sdlGLContext self',
    {
        cdef =
[[
    CHECK_META(sdlGLContext);
    SDL_GL_DeleteContext(*self);
    return 0;
]]
    }),
    }
}

--- SDL_Joystick
meta_types[6] = {
    name = 'sdlJoystick',
    typename = 'sdl.Joystick',
    ctype  = 'SDL_Joystick*',
    blocks = {
    function_block('string', 'get_name', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_STRING(SDL_JoystickName(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_vendor', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetVendor(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_product', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProduct(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_product_version', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetProductVersion(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_type', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickGetType(*self));
    return 1;
]]
    }),
    c_code_block(
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
]]
    ),
    function_block('string', 'get_type_string', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_STRING(joy_types[SDL_JoystickGetType(*self)]);
    return 1;
]]
    }),
    function_block('integer', 'num_axes', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumAxes(*self));
    return 1;
]]
    }),
    function_block('integer', 'num_balls', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumBalls(*self));
    return 1;
]]
    }),
    function_block('integer', 'num_hats', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumHats(*self));
    return 1;
]]
    }),
    function_block('integer', 'num_buttons', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickNumButtons(*self));
    return 1;
]]
    }),
    function_block('integer', 'get_axis', 'sdlJoystick self, integer axis',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(axis);
    PUSH_INTEGER(SDL_JoystickGetAxis(*self, axis));
    return 1;
]]
    }),
    function_block('integer, integer', 'get_ball', 'sdlJoystick self, integer ball',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(ball);
    int dx, dy;
    SDL_JoystickGetBall(*self, ball, &dx, &dy);
    PUSH_INTEGER(dx);
    PUSH_INTEGER(dy);
    return 2;
]]
    }),
    function_block('integer', 'get_hat', 'sdlJoystick self, integer hat',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(hat);
    PUSH_INTEGER(SDL_JoystickGetHat(*self, hat));
    return 1;
]]
    }),
    function_block('integer', 'get_button', 'sdlJoystick self, integer button',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(button);
    PUSH_BOOLEAN(SDL_JoystickGetButton(*self, button));
    return 1;
]]
    }),
    function_block('boolean', 'rumble', 'sdlJoystick self, integer low, integer high, integer|nil freq',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    CHECK_INTEGER(low);
    CHECK_INTEGER(high);
    Uint32 freq = (Uint32)luaL_optinteger(L, arg++, 100);
    PUSH_BOOLEAN(SDL_JoystickRumble(*self, low, high, freq) == 0);
    return 1;
]]
    }),
    function_block('integer', 'current_power_level', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    PUSH_INTEGER(SDL_JoystickCurrentPowerLevel(*self));
    return 1;
]]
    }),
    function_block('', 'close', 'sdlJoystick self',
    {
        cdef =
[[
    CHECK_META(sdlJoystick);
    if (SDL_JoystickGetAttached(*self))
        SDL_JoystickClose(*self);
    return 0;
]]
    }),
    }
}

--- SDL_Renderer
meta_types[7] = {
    name = 'sdlRenderer',
    typename = 'sdl.Renderer',
    ctype  = 'SDL_Renderer*',
    blocks = {
    c_code_block(
[[
#include "helper.h"
]]
    ),
    function_block('', 'destroy', 'sdlRenderer self',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    SDL_DestroyRenderer(*self);
    return 0;
]]
    }),
    function_block('', 'clear', 'sdlRenderer self',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    SDL_RenderClear(*self);
    return 0;
]]
    }),
    function_block('', 'set_color', 'sdlRenderer self, integer r, integer g, integer b, integer a',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    Uint8 d[4];
    for (int i = 0; i < 4; i++) d[i] = (Uint8)luaL_checkinteger(L, i+2);
    SDL_SetRenderDrawColor(*self, d[0], d[1], d[2], d[3]);
    return 0;
]]
    }),
    function_block('', 'set_target', 'sdlRenderer self, sdlTexture|nil tex',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    const TEST_UDATA(sdlTexture, tex);
    SDL_SetRenderTarget(*self, *tex);
    return 0;
]]
    }),
    function_block('', 'set_clip_rect', 'sdlRenderer self, table|nil rect',
    {
        cdef =
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
    }),
    function_block('', 'set_blend_mode', 'sdlRenderer self, integer blend_mode',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    CHECK_INTEGER(blend_mode);
    SDL_SetRenderDrawBlendMode(*self, blend_mode);
    return 0;
]]
    }),
    function_block('', 'draw_point', 'sdlRenderer self, number x, number y',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, x);
    const CHECK_NUMBER(float, y);
    SDL_RenderDrawPointF(*self, x, y);
    return 0;
]]
    }),
    function_block('', 'draw_line', 'sdlRenderer self, number x0, number y0, number x1, number y1',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    const CHECK_NUMBER(float, x0);
    const CHECK_NUMBER(float, y0);
    const CHECK_NUMBER(float, x1);
    const CHECK_NUMBER(float, y1);
    SDL_RenderDrawLineF(*self, x0, y0, x1, y1);
    return 0;
]]
    }),
    function_block('', 'draw_triangle', 'sdlRenderer self, number x0, number y0, number x1, number y1, number x2, number y2',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    float pos[6];
    for (int i = 0; i < 6; i++) pos[i] = (float)luaL_checknumber(L, i+2);
    SDL_RenderDrawLineF(*self, pos[0], pos[1], pos[2], pos[3]);
    SDL_RenderDrawLineF(*self, pos[2], pos[3], pos[4], pos[5]);
    SDL_RenderDrawLineF(*self, pos[4], pos[5], pos[0], pos[1]);
    return 0;
]]
    }),
    c_code_block(
[[
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
]]
    ),
    function_block('', 'fill_triangle', 'sdlRenderer self, number x0, number y0, number x1, number y1, number x2, number y2',
    {
        cdef =
[[
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
]]
    }),
    function_block('', 'draw_rect', 'sdlRenderer self, number x, number y, number w, number h',
    {
        cdef =
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
    }),
    function_block('', 'fill_rect', 'sdlRenderer self, number x, number y, number w, number h',
    {
        cdef =
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
    }),
    function_block('', 'draw_circle', 'sdlRenderer self, number x, number y, number radius, integer|nil sides',
    {
        cdef =
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
    }),
    function_block('', 'fill_circle', 'sdlRenderer self, number x, number y, number radius, integer|nil sides',
    {
        cdef =
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
    }),
    function_block('', 'copy', 'sdlRenderer self, sdlTexture tex, table|nil src, table|nil dest',
    {
        cdef =
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
    }),
    function_block('', 'copy_ex', 'sdlRenderer self, sdlTexture tex, table|nil src, table|nil dest',
    {
        cdef =
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
    }),
    function_block('', 'print', 'sdlRenderer self, sdlFont font, string text, number|nil x, number|nil y',
    {
        cdef =
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
    }),
    function_block('', 'flush', 'sdlRenderer self',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    SDL_RenderFlush(*self);
    return 0;
]]
    }),
    function_block('', 'present', 'sdlRenderer self',
    {
        cdef =
[[
    CHECK_META(sdlRenderer);
    SDL_RenderPresent(*self);
    return 0;
]]
    }),
    }
}

--- SDL_RWops
meta_types[#meta_types+1] = {
    name = 'sdlRWops',
    typename = 'sdl.RWops',
    ctype  = 'SDL_RWops*',
    blocks = {
    function_block('integer', 'read', 'sdlRWops self, lightuserdata data, integer size, integer|nil maxnum',
    {
        cdef =
[[
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(maxnum, 1);
    size_t read = SDL_RWread(self, data, size, maxnum);
    PUSH_INTEGER(read);
    return 1;
]]
    }),
    function_block('integer', 'writen', 'sdlRWops self, lightuserdata data, integer size, integer|nil num',
    {
        cdef =
[[
    CHECK_META(sdlRWops);
    CHECK_LUDATA(void, data);
    CHECK_INTEGER(size);
    OPT_INTEGER(num, 1);
    size_t written = SDL_RWwrite(self, data, size, num);
    PUSH_INTEGER(written);
    return 1;
]]
    }),
    function_block('integer', 'seek', 'sdlRWops self, integer offset, integer whence',
    {
        cdef =
[[
    CHECK_META(sdlRWops);
    CHECK_INTEGER(offset);
    OPT_INTEGER(whence, RW_SEEK_SET);
    Sint64 res = SDL_RWseek(self, offset, whence);
    PUSH_INTEGER(res);
    return 1;
]]
    }),
    function_block('integer', 'size', 'sdlRWops self',
    {
        cdef =
[[
    CHECK_META(sdlRWops);
    Sint64 size = SDL_RWsize(self);
    PUSH_INTEGER(size);
    return 1;
]]
    }),
    function_block('integer', 'tell', 'sdlRWops self',
    {
        cdef =
[[
    CHECK_META(sdlRWops);
    Sint64 offset = SDL_RWtell(self);
    PUSH_INTEGER(offset);
    return 1;
]]
    }),
    function_block('boolean', 'close', 'sdlRWops self',
    {
        cdef =
[[
    CHECK_META(sdlRWops);
    Sint64 res = SDL_RWclose(self);
    PUSH_BOOLEAN(res == 0);
    return 1;
]]
    }),
    }
}

--- SDL_Texture
meta_types[#meta_types+1] = {
    name = 'sdlTexture',
    typename = 'sdl.Texture',
    ctype  = 'SDL_Texture*',
    blocks = {
    function_block('', 'destroy', 'sdlTexture self',
    {
        cdef =
[[
    CHECK_META(sdlTexture);
    SDL_DestroyTexture(*self);
    return 0;
]]
    }),
    function_block('integer, integer, integer, integer', 'query', 'sdlTexture self',
    {
        cdef =
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
    }),
    function_block('', 'set_alpha_mod', 'sdlTexture self, integer alpha',
    {
        cdef =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(alpha);
    SDL_SetTextureAlphaMod(*self, (Uint8)alpha);
    return 0;
]]
    }),
    function_block('', 'set_color_mod', 'sdlTexture self, integer r, integer g, integer b',
    {
        cdef =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(r);
    CHECK_INTEGER(g);
    CHECK_INTEGER(b);
    SDL_SetTextureColorMod(*self, r, g, b);
    return 0;
]]
    }),
    function_block('', 'set_blend_mode', 'sdlTexture self, integer blend_mode',
    {
        cdef =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(blend_mode);
    SDL_SetTextureBlendMode(*self, blend_mode);
    return 0;
]]
    }),
    function_block('', 'set_scale_mode', 'sdlTexture self, integer scale_mode',
    {
        cdef =
[[
    CHECK_META(sdlTexture);
    CHECK_INTEGER(scale_mode);
    SDL_SetTextureScaleMode(*self, scale_mode);
    return 0;
]]
    }),
    }
}

--- SDL_Window
meta_types[#meta_types+1] = {
    name = 'sdlWindow',
    typename = 'sdl.Window',
    ctype  = 'SDL_Window*',
    blocks = {
    function_block('', 'destroy', 'sdlWindow self',
    {
        cdef =
[[
    CHECK_META(sdlWindow);
    SDL_DestroyWindow(*self);
    return 0;
]]
    }),
    function_block('integer, integer', 'get_size', 'sdlWindow self',
    {
        cdef =
[[
    INIT_ARG();
    CHECK_UDATA(sdlWindow, window);
    int width, height;
    SDL_GetWindowSize(*window, &width, &height);
    PUSH_INTEGER(width);
    PUSH_INTEGER(height);
    return 2;
]]
    }),
    function_block('', 'set_size', ''),
    function_block('integer, integer', 'get_position', 'sdlWindow self',
    {
        cdef =
[[
    CHECK_META(sdlWindow);
    int x, y;
    SDL_GetWindowPosition(*self, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
]]
    }),
    function_block('', 'set_position', ''),
    function_block('', 'gl_swap', 'sdlWindow self',
    {
        cdef =
[[
    CHECK_META(sdlWindow);
    SDL_GL_SwapWindow(*self);
    return 0;
]]
    }),
    }
}

return meta_types