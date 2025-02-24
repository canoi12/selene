#ifndef SELENE_NO_AUDIO
#include "audio.h"

typedef struct AudioSystem AudioSystem;

struct AudioBuffer {
    int type;
    int volume;
    int loop;
    int playing;
    int offset;
    SDL_AudioStream* stream;
    union {
        AudioDecoder* decoder;
        AudioData* audio_data;
    };
};

struct AudioSystem {
    int paused;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device;
    struct AudioBuffer* buffer_pool;
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
    AudioDecoder* decoder;
    AudioSystem* system;
    SDL_AudioStream* stream;
};

struct Sound {
    AudioSystem* system;
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
    AudioSystem* system = (AudioSystem*)userdata;
    //SDL_LockMutex(system->audio_mutex);
    int i = 0;
    while (i < system->pool_count) {
        struct AudioBuffer* buffer = &system->buffer_pool[i];
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
            //fprintf(stderr, "volume: %d\n", buffer->volume);
#endif
        }
        i++;
    }
    //SDL_UnlockMutex(system->audio_mutex);
}

/**
 * AudioSystem
 */

static MODULE_FUNCTION(AudioSystem, play) {
    CHECK_META(AudioSystem);
    TEST_UDATA(AudioDecoder, dec);
    arg--;
    TEST_UDATA(AudioData, adata);
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
        struct AudioBuffer* buffer = &(self->buffer_pool[i]);
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
    CHECK_META(AudioSystem);
    CHECK_INTEGER(inst);
    self->buffer_pool[inst].playing = !lua_toboolean(L, arg);
    return 0;
}

static MODULE_FUNCTION(AudioSystem, update) {
    CHECK_META(AudioSystem);
#if defined(SELENE_USE_SDL3)
    int samples = 4096 * SDL_AUDIO_FRAMESIZE(self->spec);
#endif
    if (self->paused)
        return 0;
    int i = 0;
    short* aux = (short*)self->aux_data;
    while (i < self->pool_count) {
        struct AudioBuffer* buffer = &(self->buffer_pool[i]);
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
                int res = SDL_AudioStreamPut(buffer->stream, aux, read * self->spec.channels * sizeof(short));
#endif
            }
        } else if (buffer->audio_data && buffer->type == 1) {
            int size = buffer->audio_data->size;
#if defined(SELENE_USE_SDL3)
            int len = samples;
#else
            int len = self->spec.size;
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
    CHECK_META(AudioSystem);
#if defined(SELENE_USE_SDL3)
    SDL_ResumeAudioDevice(self->device);
#else
    SDL_PauseAudioDevice(self->device, 0);
#endif
    self->paused = 0;
    return 0;
}

static MODULE_FUNCTION(AudioSystem, pause_device) {
    CHECK_META(AudioSystem);
#if defined(SELENE_USE_SDL3)
    SDL_PauseAudioDevice(self->device);
#else
    SDL_PauseAudioDevice(self->device, 1);
#endif
    self->paused = 1;
    return 0;
}

static MODULE_FUNCTION(AudioSystem, set_volume) {
    CHECK_META(AudioSystem);
    CHECK_INTEGER(sound);
    if (sound < 0 || sound >= self->pool_count)
        return luaL_error(L, "invalid sound instance");
    CHECK_NUMBER(float, volume);
    struct AudioBuffer* buf = &(self->buffer_pool[sound]);
#if defined(SELENE_USE_SDL3)
    buf->volume = volume;
#else
    buf->volume = (int)((float)SDL_MIX_MAXVOLUME * volume);
    // fprintf(stderr, "volume: %d\n", buf->volume);
#endif
    return 0;
}

static MODULE_FUNCTION(AudioSystem, set_loop) {
    CHECK_META(AudioSystem);
    CHECK_INTEGER(sound);
    if (sound < 0 || sound >= self->pool_count)
        return luaL_error(L, "invalid sound instance");
    GET_BOOLEAN(loop);
    struct AudioBuffer* buf = &(self->buffer_pool[sound]);
    buf->loop = loop;
    return 0;
}

static MODULE_FUNCTION(AudioSystem, close) {
    CHECK_META(AudioSystem);
    if (self->aux_data)
        free(self->aux_data);
    self->aux_data = NULL;
    SDL_CloseAudioDevice(self->device);
    free(self->buffer_pool);
    return 0;
}

static MODULE_FUNCTION(AudioSystem, meta) {
    BEGIN_REG(reg)
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

/**
 * Load audio data from a file (ogg, wav, mp3 and flac)
 */
static MODULE_FUNCTION(audio, load_data) {
    INIT_ARG();
    size_t len;
    CHECK_LSTRING(path, &len);
    AudioDecoder dec;
    int res = s_AudioDecoder_init(L, path, (int)len, &dec);
    if (res <= 0) {
        return luaL_error(L, "failed to load audio data: %s", path);
    }
    SDL_AudioSpec dec_spec;
    dec_spec.freq = dec.info.sample_rate;
#if defined(SELENE_USE_SDL3)
    if (dec.info.bit_depth == 16) dec_spec.format = SDL_AUDIO_S16;
    else if (dec.info.bit_depth == 32) dec_spec.format = SDL_AUDIO_F32;
#else
    if (dec.info.bit_depth == 16) dec_spec.format = AUDIO_S16;
    else if (dec.info.bit_depth == 32) dec_spec.format = AUDIO_F32;
    else dec_spec.format = SELENE_AUDIO_FORMAT;
#endif
    dec_spec.channels = dec.info.channels;
    SDL_AudioSpec spec;
    // fprintf(stderr, "format: %d\n", format);
    spec.freq = SELENE_AUDIO_SAMPLE_RATE;
    spec.format = SELENE_AUDIO_FORMAT;
    spec.channels = SELENE_AUDIO_CHANNELS;
    int samples = SELENE_AUDIO_SAMPLES;
    if (lua_type(L, arg) == LUA_TTABLE) {
        lua_getfield(L, arg, "sample_rate");
        spec.freq = (int)luaL_optinteger(L, -1, spec.freq);
        lua_pop(L, 1);
        lua_getfield(L, arg, "format");
        spec.format = (int)luaL_optinteger(L, -1, spec.format);
        lua_pop(L, 1);
        lua_getfield(L, arg, "channels");
        spec.channels = (int)luaL_optinteger(L, -1, spec.channels);
        lua_getfield(L, arg, "samples");
        samples = (int)luaL_optinteger(L, -1, samples);
        lua_pop(L, 1);
    }
    short aux_data[16384];
    int frame_count = s_AudioDecoder_read_s16(&dec, 4096, aux_data);
    if (frame_count < 0) {
        return luaL_error(L, "Failed to read audio decoder");
    }
#if defined(SELENE_USE_SDL3)
    SDL_AudioStream* stream = SDL_CreateAudioStream(&dec_spec, &spec);
#else
    SDL_AudioStream* stream = SDL_NewAudioStream(
            dec_spec.format, dec.info.channels, dec.info.sample_rate,
            spec.format, spec.channels, spec.freq);
#endif


    while (frame_count != 0) {
        // short* buffer = (short*)malloc(frame_count * channels * sizeof(short));
        short* buffer = (short*)aux_data;
#if defined(SELENE_USE_SDL3)
        int res = SDL_PutAudioStreamData(stream, buffer, frame_count * spec.channels * sizeof(short));
#else
        int res = SDL_AudioStreamPut(stream, buffer, frame_count * spec.channels * sizeof(short));
#endif
        if (res < 0) {
            return luaL_error(L, "Failed to put audio stream: %s", SDL_GetError());
        }
        frame_count = s_AudioDecoder_read_s16(&dec, samples, buffer);
        // free(buffer);
    }
#if defined(SELENE_USE_SDL3)
    size_t size = SDL_GetAudioStreamAvailable(stream);
#else
    size_t size = SDL_AudioStreamAvailable(stream);
#endif
    NEW_UDATA(AudioData, data);
    data->size = (int)size;
    data->data = malloc(size);
    data->info.sample_rate = spec.freq;
    data->info.channels = spec.channels;
    data->info.format = spec.format;
#if defined(SELENE_USE_SDL3)
    int read = SDL_GetAudioStreamData(stream, data->data, size);
#else
    int read = SDL_AudioStreamGet(stream, data->data, (int)size);
#endif
    if (read < 0) {
        return luaL_error(L, "Failed to read audio stream: %s", SDL_GetError());
    }
#if defined(SELENE_USE_SDL3)
    SDL_DestroyAudioStream(stream);
#else
    SDL_FreeAudioStream(stream);
#endif
    return 1;
}

/**
 * Load an audio decoder from a file (ogg, wav, mp3 and flac)
 */
static MODULE_FUNCTION(audio, load_decoder) {
    INIT_ARG();
    size_t len;
    CHECK_LSTRING(path, &len);

    AudioDecoder out;
    int res = s_AudioDecoder_init(L, path, (int)len, &out);
    if (res <= 0) {
        return res;
    }
    NEW_UDATA(AudioDecoder, dec);
    memcpy(dec, &out, sizeof(AudioDecoder));

    return 1;
}

static MODULE_FUNCTION(audio, create_system) {
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
    NEW_UDATA(AudioSystem, system);
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
    size = obtained.size;
    fprintf(stderr, "spec size: %d\n", size);
    if (dev == 0) {
        const char* name = SDL_GetAudioDeviceName(0, 0);
        dev = SDL_OpenAudioDevice(name, 0, &spec, &obtained, 0);
        if (dev == 0)
            return luaL_error(L, "Failed to open audio device: %s\n", SDL_GetError());
        else
            size = obtained.size;
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
    system->buffer_pool = (struct AudioBuffer*)malloc(system->pool_count * sizeof(struct AudioBuffer));
    memset(system->buffer_pool, 0, system->pool_count * sizeof(struct AudioBuffer));
    system->paused = 1;
    system->aux_data = malloc(size);

    return 1;
}

extern int l_AudioDecoder_meta(lua_State* L);

int luaopen_audio(lua_State* L) {
    BEGIN_REG(reg)
        REG_FIELD(audio, create_system),
        REG_FIELD(audio, load_data),
        REG_FIELD(audio, load_decoder),
    END_REG()
    luaL_newlib(L, reg);
    luaL_newmetatable(L, "AudioData");
    lua_setfield(L, -2, "AudioData");
    l_AudioDecoder_meta(L);
    lua_setfield(L, -2, "AudioDecoder");
    l_AudioSystem_meta(L);
    lua_setfield(L, -2, "AudioSystem");
    return 1;
}

#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#endif /* SELENE_NO_AUDIO */
