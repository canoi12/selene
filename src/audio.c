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

typedef struct {
    Uint8 audio_type;
    Uint32 size;
    void* data;
    Uint32 frequency;
    Uint32 samples;
    Uint8 channels;
} AudioData;

typedef struct AudioSource AudioSource;
struct AudioSource {
    Data data;
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
    AudioSource* source;
};

typedef struct {
    AudioBuffer* data;
    Uint32 size;
    int* available;
    int top;
} BufferPool;
static BufferPool* _buffer_pool = NULL;

static void _audio_callback(void* userdata, Uint8* stream, int len) {
    #if 1
    SDL_memset(stream, 0, len);
    for (int i = 0; i < _buffer_pool->size; i++) {
        int l = len;
        AudioBuffer* buf = &_buffer_pool->data[i];
        if (!buf->playing)
            continue;
        int offset, size;
        offset = buf->offset;
        size = buf->source->data.size;
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
            stream[j] += data[j];
        }
    }
    #endif
}

static BEGIN_FUNCTION(audio, GetCallback)
    PUSH_LUDATA(_audio_callback);
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadWav)
    CHECK_STRING(path);
    Uint32 channels, sample_rate;
    drwav_uint64 frame_count;
    Sint16* frames = drwav_open_file_and_read_pcm_frames_s16(path, &channels, &sample_rate, &frame_count, NULL);
    if (!frames) {
        return luaL_error(L, "Failed to load wav");
    }

    NEW_UDATA(AudioSource, source);
    source->data.data = frames;
    source->data.size = frame_count * channels * sizeof(Sint16);
    source->frequency = sample_rate;
    source->channels = channels;
    source->samples = 4096;
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadOgg)
    CHECK_STRING(path);
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, Play)
    CHECK_UDATA(AudioSource, source);
    if (!_buffer_pool) {
        return luaL_error(L, "Audio buffer pool not initialized");
    }
    AudioBuffer* b = _buffer_pool->data;
    b->volume = 100;
    b->pitch = 100;
    b->offset = 0;
    b->source = source;
    b->playing = 1;
END_FUNCTION(1)

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
    b->volume = 100;
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
    b->playing = pause;
END_FUNCTION(0)

static BEGIN_META_FUNCTION(BufferPool, SetCurrent)
    _buffer_pool = self;
END_FUNCTION(0)

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
        REG_META_FIELD(BufferPool, SetCurrent),
        REG_META_FIELD(BufferPool, Get),
        REG_META_FIELD(BufferPool, GetSize),
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
        REG_FIELD(audio, Play),
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
