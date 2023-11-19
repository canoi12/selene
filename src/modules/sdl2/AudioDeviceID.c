#include "sdl2.h"

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

static MODULE_FUNCTION(AudioDeviceID, open) {
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
    #if 0
    lua_getfield(L, arg, "callback");
    void* callback = NULL;
    if (lua_type(L, -1) == LUA_TLIGHTUSERDATA)
        callback = lua_touserdata(L, -1);
    #endif
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
    pool->data = malloc(pool_size * sizeof(AudioStream));
    memset(pool->data, 0, pool_size * sizeof(AudioStream));
    pool->top = 0;
    pool->availables = malloc(pool_size * sizeof(int));
    for (int i = 0; i < pool_size; i++) {
        pool->availables[i] = pool_size - i - 1;
    }
    pool->top = pool_size;

    GET_BOOLEAN(allowed_changes);
    AudioDeviceID d = SDL_OpenAudioDevice(device, is_capture, &desired, &obtained, allowed_changes);
    if (d == 0) {
        PUSH_NIL();
        return 1;
    } else {
        NEW_UDATA(AudioDeviceID, dev);
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

static MODULE_FUNCTION(AudioDeviceID, getCount) {
    INIT_ARG();
    GET_BOOLEAN(is_capture);
    PUSH_INTEGER(SDL_GetNumAudioDevices(is_capture));
    return 1;
}

static MODULE_FUNCTION(AudioDeviceID, getName) {
    INIT_ARG();
    CHECK_INTEGER(index);
    GET_BOOLEAN(is_capture);
    PUSH_STRING(SDL_GetAudioDeviceName(index, is_capture));
    return 1;
}

// META

static META_FUNCTION(AudioDeviceID, pause) {
    CHECK_META(AudioDeviceID);
    GET_BOOLEAN(pause);
    SDL_PauseAudioDevice(*self, pause);
    return 0;
}

static META_FUNCTION(AudioDeviceID, close) {
    CHECK_META(AudioDeviceID);
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

BEGIN_META(AudioDeviceID) {
    BEGIN_REG()
        REG_FIELD(AudioDeviceID, open),
        REG_FIELD(AudioDeviceID, getCount),
        REG_FIELD(AudioDeviceID, getName),
    END_REG()

    BEGIN_REG(_index)
        REG_META_FIELD(AudioDeviceID, pause),
        REG_META_FIELD(AudioDeviceID, close),
    END_REG()
    NEW_META(AudioDeviceID, _reg, _index_reg);
    return 1;
}
