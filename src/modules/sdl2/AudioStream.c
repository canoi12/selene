#include "lua.h"
#include "sdl2.h"

static MODULE_FUNCTION(AudioStream, create) {
    INIT_ARG();
    CHECK_INTEGER(src_format);
    CHECK_INTEGER(src_channels);
    CHECK_INTEGER(src_rate);
    CHECK_INTEGER(dest_format);
    CHECK_INTEGER(dest_channels);
    CHECK_INTEGER(dest_rate);

    SDL_AudioStream* s = SDL_NewAudioStream(src_format, src_channels, src_rate, dest_format, dest_channels, dest_rate);
    if (!s)
        return luaL_error(L, "Failed to create audio stream: %s\n", SDL_GetError()); 
    NEW_UDATA(AudioStream, stream);
    *stream = s;
    return 1;
}

// Meta

static META_FUNCTION(AudioStream, clear) {
    CHECK_META(AudioStream);
    SDL_AudioStreamClear(*self);
    return 0;
}

static META_FUNCTION(AudioStream, free) {
    CHECK_META(AudioStream);
    SDL_FreeAudioStream(*self);
    return 0;
}

static META_FUNCTION(AudioStream, flush) {
    CHECK_META(AudioStream);
    SDL_AudioStreamFlush(*self);
    return 0;
}

static META_FUNCTION(AudioStream, bind) {
    CHECK_META(AudioStream);
    CHECK_UDATA(AudioDeviceID, dev);
    PUSH_VALUE(-1);
    lua_rawget(L, LUA_REGISTRYINDEX);
    AudioStreamPool* pool = lua_touserdata(L, -1);
    if (pool->top <= 0)
        return luaL_error(L, "AudioStreamPool is full\n");
    int index = pool->availables[--(pool->top)];
    pool->data[index] = *self;
    return 0;
}

static META_FUNCTION(AudioStream, unbind) {
    CHECK_META(AudioStream);
    CHECK_UDATA(AudioDeviceID, dev);
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
}

static META_FUNCTION(AudioStream, put) {
    CHECK_META(AudioStream);
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
            data = d->data;
            len = d->size;
        }
        break;
    }
    int res = SDL_AudioStreamPut(*self, data, (int)len);
    PUSH_INTEGER(res);
    return 1;
}

static META_FUNCTION(AudioStream, get) {
    CHECK_META(AudioStream);
    CHECK_UDATA(Data, out);
    OPT_INTEGER(size, out->size);
    int res = SDL_AudioStreamGet(*self, out->data, size);
    PUSH_INTEGER(res);
    return 1;
}

static META_FUNCTION(AudioStream, available) {
    CHECK_META(AudioStream);
    int res = SDL_AudioStreamAvailable(*self);
    PUSH_INTEGER(res);
    return 1;
}

BEGIN_META(AudioStream) {
    BEGIN_REG()
        REG_FIELD(AudioStream, create),
    END_REG()
    BEGIN_REG(_index)
        REG_META_FIELD(AudioStream, clear),
        REG_META_FIELD(AudioStream, free),
        REG_META_FIELD(AudioStream, flush),
        REG_META_FIELD(AudioStream, bind),
        REG_META_FIELD(AudioStream, unbind),
        REG_META_FIELD(AudioStream, put),
        REG_META_FIELD(AudioStream, get),
        REG_META_FIELD(AudioStream, available),
    END_REG()
    NEW_META(AudioStream, _reg, _index_reg);
    return 1;
}
