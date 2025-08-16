#include "modules/audio.h"
#ifndef SELENE_NO_AUDIO
struct _AudioBuffer {
    int type;
    int volume;
    int loop;
    int playing;
    int offset;
    SDL_AudioStream* stream;
    union {
        selene_AudioDecoder* decoder;
        selene_AudioData* audio_data;
    };
};

struct _AudioSystem {
    int paused;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device;
    struct _AudioBuffer* buffer_pool;
    int pool_count;
    void* aux_data;
#if defined(SELENE_USE_SDL3)
    SDL_Mutex* audio_mutex;
#else
    SDL_mutex* audio_mutex;
#endif
    int audio_format;
};

struct Music {
    selene_AudioDecoder* decoder;
    selene_AudioSystem* system;
    SDL_AudioStream* stream;
};

struct Sound {
    selene_AudioSystem* system;
    SDL_AudioStream* stream;
    void* data;
    int size;
};

static void s_sdl_audio_callback(void* userdata, Uint8* stream, int len) {
#if defined(OS_WIN)
    Uint8 temp[32000];
#else
    Uint8 temp[len];
#endif
    // fprintf(stderr, "AudioCallback\n");
    SDL_memset(stream, 0, len);
    selene_AudioSystem* system = (selene_AudioSystem*)userdata;
    //SDL_LockMutex(system->audio_mutex);
    int i = 0;
    while (i < system->pool_count) {
        struct _AudioBuffer* buffer = &system->buffer_pool[i];
        if (buffer->stream && buffer->playing) {
#if defined(SELENE_USE_SDL3)
            int result = SDL_GetAudioStreamData(buffer->stream, temp, len);
#else
            int result = SDL_AudioStreamGet(buffer->stream, temp, len);
#endif
            if (result < 0) {}
            else if (result != len) {
                SDL_memset(temp + result, 0, len - result);
            }
#if defined(SELENE_USE_SDL3)
            SDL_MixAudio(stream, temp, SELENE_AUDIO_FORMAT, len, buffer->volume);
#else
            SDL_MixAudioFormat(stream, temp, SELENE_AUDIO_FORMAT, len, buffer->volume);
#endif
        }
        i++;
    }
    //SDL_UnlockMutex(system->audio_mutex);
}

/**
 * AudioSystem
 */

MODULE_FUNCTION(AudioSystem, create) {
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
            return luaL_error(L, "failed to init SDL Audio subsystem: %s", SDL_GetError());
    }
    int freq = SELENE_AUDIO_SAMPLE_RATE;
    int format = SELENE_AUDIO_FORMAT;
    int channels = SELENE_AUDIO_CHANNELS;
    int samples = SELENE_AUDIO_SAMPLES;
    INIT_ARG();
    if (lua_type(L, arg) == LUA_TTABLE) {
        lua_getfield(L, arg, "sample_rate");
        freq = (int)luaL_optinteger(L, -1, freq);
        lua_pop(L, 1);
        lua_getfield(L, arg, "format");
        format = (int)luaL_optinteger(L, -1, format);
        lua_pop(L, 1);
        lua_getfield(L, arg, "channels");
        channels = (int)luaL_optinteger(L, -1, channels);
        lua_pop(L, 1);
        lua_getfield(L, arg, "samples");
        samples = (int)luaL_optinteger(L, -1, samples);
        lua_pop(L, 1);
    }
    NEW_UDATA(selene_AudioSystem, system);
    SDL_AudioSpec spec;
    spec.freq = freq;
    spec.format = format;
    spec.channels = channels;
    int size;
#if !defined(SELENE_USE_SDL3)
    spec.samples = samples;
    spec.userdata = system;
    spec.callback = s_sdl_audio_callback;
    SDL_AudioSpec obtained;
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, 0);
    fprintf(stderr, "audio device: %d\n", dev);
    size = (int)obtained.size;
    fprintf(stderr, "spec size: %d\n", size);
    if (dev == 0) {
        const char* name = SDL_GetAudioDeviceName(0, 0);
        dev = SDL_OpenAudioDevice(name, 0, &spec, &obtained, 0);
        if (dev == 0)
            return luaL_error(L, "Failed to open audio device: %s\n", SDL_GetError());
        else
            size = (int)obtained.size;
    }
#else
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    SDL_AudioSpec obtained;
    memcpy(&obtained, &spec, sizeof(SDL_AudioSpec));
#endif

    // fprintf(stdout, "obtained: %d %d %d %d %d\n", obtained.freq, obtained.format, obtained.channels, obtained.samples, obtained.size);

    memcpy(&(system->spec), &obtained, sizeof(SDL_AudioSpec));
    system->device = dev;
    system->pool_count = 256;
    system->buffer_pool = (struct _AudioBuffer*)malloc(system->pool_count * sizeof(struct _AudioBuffer));
    memset(system->buffer_pool, 0, system->pool_count * sizeof(struct _AudioBuffer));
    system->paused = 1;
    system->aux_data = malloc(size);

    return 1;
}

static MODULE_FUNCTION(AudioSystem, play) {
    CHECK_META(selene_AudioSystem);
    TEST_UDATA(selene_AudioDecoder, dec);
    arg--;
    TEST_UDATA(selene_AudioData, adata);
    if (!dec && !adata)
        return luaL_error(L, "invalid audio data type");
    AudioInfo* info = NULL;
    if (dec) info = (AudioInfo*)dec;
    else info = (AudioInfo*)adata;
    GET_BOOLEAN(loop);
    OPT_NUMBER(float, volume, 1.f);
    SDL_AudioSpec spec;
    spec.freq = info->sample_rate;
#if defined(SELENE_USE_SDL3)
    if (info->bit_depth == 16) spec.format = SDL_AUDIO_S16;
    else if (info->bit_depth == 32) spec.format = SDL_AUDIO_F32;
    else spec.format = SELENE_AUDIO_FORMAT;
#else
    if (info->bit_depth == 16) spec.format = AUDIO_S16SYS;
    else if (info->bit_depth == 32) spec.format = AUDIO_F32;
    else spec.format = SELENE_AUDIO_FORMAT;
#endif
    spec.channels = info->channels;

    int i = 0;
    while (i < self->pool_count) {
        struct _AudioBuffer* buffer = &(self->buffer_pool[i]);
        if (!buffer->stream) {
            buffer->playing = 1;
            buffer->loop = loop;
#if defined(SELENE_USE_SDL3)
            buffer->volume = volume;
#else
            buffer->volume = (int)((float)SDL_MIX_MAXVOLUME * volume);
#endif
            self->buffer_pool[i].playing = 1;
            if (dec) {
                buffer->decoder = dec;
                buffer->offset = 0;
                buffer->type = 0;
                spec.channels = dec->info.channels;
                spec.freq = dec->info.sample_rate;
#if defined(SELENE_USE_SDL3)
                if (dec->info.bit_depth == 16) spec.format = SDL_AUDIO_S16;
                else if (dec->info.bit_depth == 32) spec.format = SDL_AUDIO_F32;
                else spec.format = SELENE_AUDIO_FORMAT;
#else
                if (dec->info.bit_depth == 16) spec.format = AUDIO_S16SYS;
                else if (dec->info.bit_depth == 32) spec.format = AUDIO_F32;
                else spec.format = SELENE_AUDIO_FORMAT;
#endif
            } else if (adata) {
                buffer->audio_data = adata;
                buffer->offset = 0;
                buffer->type = 1;
                spec.freq = adata->info.sample_rate;
                spec.channels = adata->info.channels;
#if defined(SELENE_USE_SDL3)
                if (adata->info.bit_depth == 16) spec.format = SDL_AUDIO_S16;
                else if (adata->info.bit_depth == 32) spec.format = SDL_AUDIO_F32;
                else spec.format = SELENE_AUDIO_FORMAT;
#else
                if (adata->info.bit_depth == 16) spec.format = AUDIO_S16SYS;
                else if (adata->info.bit_depth == 32) spec.format = AUDIO_F32;
                else spec.format = SELENE_AUDIO_FORMAT;
#endif
            }

#if defined(SELENE_USE_SDL3)
            buffer->stream = SDL_CreateAudioStream(&spec, &(self->spec));
#else
            buffer->stream = SDL_NewAudioStream(
                    spec.format, spec.channels, spec.freq,
                    self->spec.format, self->spec.channels, self->spec.freq);
#endif
            break;
        }
        i++;
    }
    if (i == self->pool_count)
        return 0;
    lua_pushinteger(L, i);
    return 1;
}

static MODULE_FUNCTION(AudioSystem, pause) {
    CHECK_META(selene_AudioSystem);
    CHECK_INTEGER(inst);
    self->buffer_pool[inst].playing = !lua_toboolean(L, arg);
    return 0;
}

static MODULE_FUNCTION(AudioSystem, update) {
    CHECK_META(selene_AudioSystem);
#if defined(SELENE_USE_SDL3)
    int samples = 4096 * SDL_AUDIO_FRAMESIZE(self->spec);
#endif
    if (self->paused)
        return 0;
    int i = 0;
    short* aux = (short*)self->aux_data;
    while (i < self->pool_count) {
        struct _AudioBuffer* buffer = &(self->buffer_pool[i]);
        if (buffer->decoder && buffer->type == 0) {
#if defined(SELENE_USE_SDL3)
            int read = s_AudioDecoder_read_s16(buffer->decoder, samples, aux);
#else
            int read = s_AudioDecoder_read_s16(buffer->decoder, self->spec.samples, aux);
            //fprintf(stderr, "read(%d) samples(%d)\n", read, self->spec.samples);
#endif
            if (read == 0) {
#if defined(SELENE_USE_SDL3)
                int wait = SDL_GetAudioStreamAvailable(buffer->stream);
#else
                int wait = SDL_AudioStreamAvailable(buffer->stream);
#endif
                if (wait == 0) {
                    if (buffer->loop) s_AudioDecoder_seek(buffer->decoder, 0);
                    else {
#if defined(SELENE_USE_SDL3)
                        SDL_DestroyAudioStream(buffer->stream);
#else
                        SDL_FreeAudioStream(buffer->stream);
#endif
                        buffer->stream = NULL;
                        buffer->decoder = NULL;
                    }
                }
            } else if (read > 0) {
#if defined(SELENE_USE_SDL3)
                int res = SDL_PutAudioStreamData(buffer->stream, aux, read * self->spec.channels * sizeof(short));
#else
                int res = SDL_AudioStreamPut(buffer->stream, aux, (int)(read * self->spec.channels * sizeof(short)));
#endif
            }
        } else if (buffer->audio_data && buffer->type == 1) {
            int size = buffer->audio_data->size;
#if defined(SELENE_USE_SDL3)
            int len = samples;
#else
            int len = (int)self->spec.size;
#endif
            if (buffer->offset + len > size) {
                len = size - buffer->offset;
            }
            int res = 0;
            if (len != 0) {
                char* ptr = buffer->audio_data->data + buffer->offset;
#if defined(SELENE_USE_SDL3)
                res = SDL_PutAudioStreamData(buffer->stream, ptr, len);
#else
                res = SDL_AudioStreamPut(buffer->stream, ptr, len);
#endif
                buffer->offset += len;
            }
#if defined(SELENE_USE_SDL3)
            if (SDL_GetAudioStreamAvailable(buffer->stream) == 0) {
#else
            if (SDL_AudioStreamAvailable(buffer->stream) == 0) {
#endif
                if (buffer->loop) buffer->offset = 0;
                else {
#if defined(SELENE_USE_SDL3)
                    SDL_DestroyAudioStream(buffer->stream);
#else
                    SDL_FreeAudioStream(buffer->stream);
#endif
                    buffer->stream = NULL;
                    buffer->audio_data = NULL;
                }
            }
        }
        i++;
    }
    return 0;
}

static MODULE_FUNCTION(AudioSystem, resume_device) {
    CHECK_META(selene_AudioSystem);
#if defined(SELENE_USE_SDL3)
    SDL_ResumeAudioDevice(self->device);
#else
    SDL_PauseAudioDevice(self->device, 0);
#endif
    self->paused = 0;
    return 0;
}

static MODULE_FUNCTION(AudioSystem, pause_device) {
    CHECK_META(selene_AudioSystem);
#if defined(SELENE_USE_SDL3)
    SDL_PauseAudioDevice(self->device);
#else
    SDL_PauseAudioDevice(self->device, 1);
#endif
    self->paused = 1;
    return 0;
}

static MODULE_FUNCTION(AudioSystem, set_volume) {
    CHECK_META(selene_AudioSystem);
    CHECK_INTEGER(sound);
    if (sound < 0 || sound >= self->pool_count)
        return luaL_error(L, "invalid sound instance");
    CHECK_NUMBER(float, volume);
    struct _AudioBuffer* buf = &(self->buffer_pool[sound]);
#if defined(SELENE_USE_SDL3)
    buf->volume = volume;
#else
    buf->volume = (int)((float)SDL_MIX_MAXVOLUME * volume);
    // fprintf(stderr, "volume: %d\n", buf->volume);
#endif
    return 0;
}

static MODULE_FUNCTION(AudioSystem, set_loop) {
    CHECK_META(selene_AudioSystem);
    CHECK_INTEGER(sound);
    if (sound < 0 || sound >= self->pool_count)
        return luaL_error(L, "invalid sound instance");
    GET_BOOLEAN(loop);
    struct _AudioBuffer* buf = &(self->buffer_pool[sound]);
    buf->loop = loop;
    return 0;
}

static MODULE_FUNCTION(AudioSystem, close) {
    CHECK_META(selene_AudioSystem);
    if (self->aux_data)
        free(self->aux_data);
    self->aux_data = NULL;
    SDL_CloseAudioDevice(self->device);
    free(self->buffer_pool);
    return 0;
}

MODULE_FUNCTION(AudioSystem, meta) {
    BEGIN_REG(reg)
        REG_FIELD(AudioSystem, create),
        REG_FIELD(AudioSystem, play),
        REG_FIELD(AudioSystem, pause),
        REG_FIELD(AudioSystem, resume_device),
        REG_FIELD(AudioSystem, pause_device),
        REG_FIELD(AudioSystem, update),
        REG_FIELD(AudioSystem, set_volume),
        REG_FIELD(AudioSystem, set_loop),
        REG_FIELD(AudioSystem, close),
    END_REG()
    luaL_newmetatable(L, "AudioSystem");
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
#endif