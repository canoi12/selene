#include "selene.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

static int l_audio_engine;

enum {
    UNKNOWN_FORMAT = 0,
    WAV_FORMAT,
    OGG_FORMAT,
};

enum {
    AUDIO_STATIC = 0,
    AUDIO_STREAM
};

typedef struct {
    Uint8 audio_type;
    Uint32 size;
    void* data;
    Uint32 frequency;
    Uint32 samples;
    Uint8 channels;
} AudioData;

typedef struct {
    char format;
    union {
        drwav wav;
        stb_vorbis ogg;
    };
    Uint32 sample_rate;
    Uint8 bit_depth;
    Uint8 channels;
} Decoder;

typedef struct {
    Data data;
    Uint32 sample_rate;
    Uint8 bit_depth;
    Uint8 channels;
} AudioData;

typedef struct AudioSource AudioSource;
struct AudioSource {
    char usage;
    union {
        Decoder decoder;
        Data data;
    };
    Uint32 frequency;
    Uint32 samples;
    Uint8 channels;
};

typedef struct AudioBuffer AudioBuffer;
struct AudioBuffer {
    char playing;
    char loop;
    int offset;
    float volume, pitch;
    SDL_AudioStream* stream;
    union {
        Decoder* decoder;
        AudioData* data;
    };
};

typedef struct {
    AudioBuffer* data;
    Uint32 size;
    int* available;
    int top;
} BufferPool;
static BufferPool* _buffer_pool = NULL;

static void _audio_callback(void* userdata, Uint8* stream, int len) {
    #if 0
    SDL_memset(stream, 0, len);
    for (int i = 0; i < _buffer_pool->size; i++) {
        int l = len;
        AudioBuffer* buf = &_buffer_pool->data[i];
        if (!buf->playing)
            continue;
        int offset, size;
        offset = buf->offset;
        SDL_AudioStreamGet(buf->stream, stream, len);
        if (offset + len >= size) {
            l = size - offset;
            buf->offset = 0;
            if (!buf->loop) {
                buf->playing = 0;
                _buffer_pool->available[_buffer_pool->top++] = (int)(buf - _buffer_pool->data);
            }
        } else {
            buf->offset += len;
        }
        char* data = (char*)buf->source->data.data + offset;
        for (int j = 0; j < l; j++) {
            stream[j] += buf->volume * data[j];
        }
    }
    #endif
}

static BEGIN_FUNCTION(audio, NewDecoder)
    CHECK_STRING(path);
    int len = strlen(path);
    char* p = path + len;
    while (*p != '.') {
        p--;
    }
    int format = UNKNOWN_FORMAT;
    if (!strcmp(p, ".ogg"))
        format = OGG_FORMAT;
    else if (!strcmp(p, ".wav"))
        format = WAV_FORMAT;
    else
        return luaL_error(L, "Unsupported audio format: %s", path);

    NEW_UDATA(Decoder, dec);
    dec->format = format;
    switch (format) {
        case WAV_FORMAT: {
            int success = drwav_init_file(&(dec->wav), path, NULL);
            if (!success)
                return luaL_error(L, "Failed to load wav: %s", path);
            dec->bit_depth = 16;
            dec->sample_rate = dec->wav.sampleRate;
            dec->channels = dec->channels;
        }
        break;
        case OGG_FORMAT: {
            stb_vorbis* ogg = stb_vorbis_open_filename(path, NULL, NULL);
            if (!ogg)
                return luaL_error(L, "Failed to load ogg: %s", path);
            stb_vorbis_info info;
            stb_vorbis_get_info(&ogg);
            dec->bit_depth = 16;
            dec->sample_rate = info.sample_rate;
            dec->channels = info.channels;
        }
        break;
    }
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, NewData)
    CHECK_STRING(path);
    int len = strlen(path);
    char* p = path + len;
    while (*p != '.') {
        p--;
    }
    int format = UNKNOWN_FORMAT;
    if (!strcmp(p, ".ogg"))
        format = OGG_FORMAT;
    else if (!strcmp(p, ".wav"))
        format = WAV_FORMAT;
    else
        return luaL_error(L, "Unsupported audio format: %s", path);

    NEW_UDATA(AudioData, dt);
    switch (format) {
        case WAV_FORMAT: {
            Uint32 channels, sample_rate;
            drwav_uint64 frame_count;
            Sint16* frames = drwav_open_file_and_read_pcm_frames_s16(path, &channels, &sample_rate, &frame_count, NULL);
            if (!frames) {
                return luaL_error(L, "Failed to load wav: %s", path);
            }

            dt->channels = channels;
            dt->bit_depth = 16;
            dt->sample_rate = sample_rate;
            dt->data.data = malloc(frame_count * sizeof(Sint16));
            dt->data.size = frame_count;
        }
        break;
        case OGG_FORMAT: {
            int channels, sample_rate;
            short* output;
            int frame_count = stb_vorbis_decode_filename(path, &channels, &sample_rate, &output);
            if (frame_count <= 0)
                return luaL_error(L, "Failed to load ogg: %s", path);

            dt->channels = channels;
            dt->bit_depth = 16;
            dt->sample_rate = sample_rate;
            dt->data.data = malloc(frame_count * sizeof(Sint16));
            dt->data.size = frame_count;
        }
        break;
    }
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, NewSource)
    CHECK_STRING(path);
    const char* usage = luaL_optstring(L, arg++, "static");
    int u = AUDIO_STATIC;
    if (!strcmp(usage, "stream"))
        u = AUDIO_STREAM;
    else if (strcmp(usage, "static"))
        return luaL_error(L, "Invalid audio usage");
    int len = strlen(path);
    char* p = path + len;
    while (p != '.') {
        p--;
    }

    int format = UNKNOWN_FORMAT;
    if (!strcmp(p, ".ogg"))
        format = OGG_FORMAT;
    else if (!strcmp(p, ".wav"))
        format = WAV_FORMAT;
    else
        return luaL_error(L, "Unsupported audio format: %s", path);


END_FUNCTION(2)

static BEGIN_FUNCTION(audio, GetCallback)
    PUSH_LUDATA(_audio_callback);
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadWav)
    CHECK_STRING(path);
    Uint32 channels, sample_rate;
    drwav_uint64 frame_count;
    drwav wav;
    drwav_init_file(&wav, path, NULL);
    Sint16* frames = drwav_open_file_and_read_pcm_frames_s16(path, &channels, &sample_rate, &frame_count, NULL);
    if (!frames) {
        return luaL_error(L, "Failed to load wav: %s", path);
    }
    Sint16 buffer[4096 * 2];
    drwav_read_pcm_frames_s16(&wav, 4096, buffer);

    NEW_UDATA(AudioSource, source);
    source->data.data = frames;
    source->data.size = frame_count * channels * sizeof(Sint16);
    source->frequency = sample_rate;
    source->channels = channels;
    source->samples = 4096;
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadOgg)
    CHECK_STRING(path);
    int channels, sample_rate;
    short* output;
    int len = stb_vorbis_decode_filename(path, &channels, &sample_rate, &output);
    if (len <= 0) {
        return luaL_error(L, "Failed to load ogg: %s", path);
    }
    
    NEW_UDATA(AudioSource, source);
    source->data.data = output;
    source->data.size = len * channels;
    source->frequency = sample_rate;
    source->channels = channels;
    source->samples = 4096;
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, SetBufferPool)
    CHECK_UDATA(BufferPool, pool);
    _buffer_pool = pool;
END_FUNCTION(0)

// Meta Functions

// Buffer Pool

static BEGIN_FUNCTION(audio, NewBufferPool)
    CHECK_INTEGER(size);
    NEW_UDATA(BufferPool, pool);
    pool->size = size;
    pool->data = malloc(sizeof(AudioBuffer) * size);
    pool->available = malloc(sizeof(int) * size);
    SDL_memset(pool->data, 0, sizeof(AudioBuffer) * size);
    for (int i = 0; i < size; i++) {
        pool->available[i] = size - i - 1;
    }
    pool->top = size;
END_FUNCTION(1)

static BEGIN_META_FUNCTION(BufferPool, Play)
    CHECK_UDATA(AudioSource, source);
    if (self->top <= 0)
        return luaL_error(L, "No free audio buffers");
    AudioBuffer* b = self->data + (self->available[--self->top]);
    b->volume = 1.f;
    b->loop = 0;
    b->pitch = 100;
    b->offset = 0;
    b->source = source;
    b->playing = 1;
    lua_pushlightuserdata(L, b);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(BufferPool, Stop)
    GET_LUDATA(AudioBuffer, b);
    int index = b - self->data;
    if (index < 0 || index >= self->size)
        return luaL_error(L, "Invalid audio buffer");
    b->playing = 0;
    self->available[self->top++] = index;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(BufferPool, Pause)
    GET_LUDATA(AudioBuffer, b);
    GET_BOOLEAN(pause);
    int index = b - self->data;
    if (index < 0 || index >= self->size)
        return luaL_error(L, "Invalid audio buffer");
    b->playing = !pause;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(BufferPool, IsPlaying)
    GET_LUDATA(AudioBuffer, b);
    int index = b - self->data;
    if (index < 0 || index >= self->size)
        return luaL_error(L, "Invalid audio buffer");
    PUSH_BOOLEAN(b->playing);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(BufferPool, Get)
    CHECK_INTEGER(index);
    if (index > self->size)
        return luaL_error(L, "Invalid pool index");
    AudioBuffer* b = self->data + index;
    lua_pushlightuserdata(L, b);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(BufferPool, GetSize)
    PUSH_INTEGER(self->size);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(BufferPool, Free)
    if (self->data && self->size > 0) {
        free(self->data);
        free(self->available);
        self->data = NULL;
        self->size = 0;
    }
END_FUNCTION(0)

static BEGIN_META(BufferPool)
    BEGIN_REG(BufferPool)
        REG_META_FIELD(BufferPool, Play),
        REG_META_FIELD(BufferPool, Stop),
        REG_META_FIELD(BufferPool, Pause),
        REG_META_FIELD(BufferPool, IsPlaying),
        REG_META_FIELD(BufferPool, Get),
        REG_META_FIELD(BufferPool, GetSize),
    // SetVolume
    // GetVolume
    // SetPitch
    // GetPitch
    // SetLoop
    // GetLoop
        REG_META_FIELD(BufferPool, Free),
    END_REG()
    NEW_META(BufferPool);
END_META(1)

// Audio Source
static BEGIN_META_FUNCTION(AudioSource, _gc)
    if (self->data.data) {
        free(self->data.data);
        self->data.data = NULL;
        self->data.size = 0;
    }
END_FUNCTION(0)

static BEGIN_META_FUNCTION(AudioSource, GetFrequency)
    PUSH_INTEGER(self->frequency);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(AudioSource, GetSamples)
    PUSH_INTEGER(self->samples);
END_FUNCTION(1)

static BEGIN_META_FUNCTION(AudioSource, GetChannels)
    PUSH_INTEGER(self->channels);
END_FUNCTION(1)

static BEGIN_META(AudioSource)
    BEGIN_REG(AudioSource)
        REG_META_FIELD(AudioSource, GetFrequency),
        REG_META_FIELD(AudioSource, GetSamples),
        REG_META_FIELD(AudioSource, GetChannels),
        REG_FIELD(AudioSource, __gc),
    END_REG()
    NEW_META(AudioSource);
END_META(1)

BEGIN_MODULE(audio)
    BEGIN_REG(audio)
        REG_FIELD(audio, NewBufferPool),
        REG_FIELD(audio, SetBufferPool),
        REG_FIELD(audio, GetCallback),
        REG_FIELD(audio, LoadOgg),
        REG_FIELD(audio, LoadWav),
    END_REG()
    NEW_MODULE(audio);
    LOAD_META(AudioSource);
    LOAD_META(BufferPool);
END_MODULE(1)


#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"
