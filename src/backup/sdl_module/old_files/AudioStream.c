#include "selene_sdl.h"

static MODULE_FUNCTION(sdlAudioStream, create) {
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

// Meta

static META_FUNCTION(sdlAudioStream, clear) {
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamClear(*self);
    return 0;
}

static META_FUNCTION(sdlAudioStream, free) {
    CHECK_META(sdlAudioStream);
    SDL_FreeAudioStream(*self);
    return 0;
}

static META_FUNCTION(sdlAudioStream, flush) {
    CHECK_META(sdlAudioStream);
    SDL_AudioStreamFlush(*self);
    return 0;
}

static META_FUNCTION(sdlAudioStream, bind) {
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

static META_FUNCTION(sdlAudioStream, unbind) {
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

static META_FUNCTION(sdlAudioStream, put) {
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
}

static META_FUNCTION(sdlAudioStream, get) {
    CHECK_META(sdlAudioStream);
    CHECK_UDATA(Data, out);
    OPT_INTEGER(size, out->size);
    int res = SDL_AudioStreamGet(*self, out->root, size);
    PUSH_INTEGER(res);
    return 1;
}

static META_FUNCTION(sdlAudioStream, available) {
    CHECK_META(sdlAudioStream);
    int res = SDL_AudioStreamAvailable(*self);
    PUSH_INTEGER(res);
    return 1;
}

BEGIN_META(sdlAudioStream) {
    BEGIN_REG(reg)
        REG_FIELD(sdlAudioStream, create),
    END_REG()
    BEGIN_REG(index_reg)
        REG_META_FIELD(sdlAudioStream, clear),
        REG_META_FIELD(sdlAudioStream, free),
        REG_META_FIELD(sdlAudioStream, flush),
        REG_META_FIELD(sdlAudioStream, bind),
        REG_META_FIELD(sdlAudioStream, unbind),
        REG_META_FIELD(sdlAudioStream, put),
        REG_META_FIELD(sdlAudioStream, get),
        REG_META_FIELD(sdlAudioStream, available),
    END_REG()
    NEW_META(sdlAudioStream, reg, index_reg);
    return 1;
}
