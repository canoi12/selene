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
    float volume, pitch;
    struct AudioSource* source;
};

typedef struct BufferPool BufferPool;
struct BufferPool {
    struct AudioBuffer* pool;
    Uint32 size;
};

int sample_pos;
float samples_per_sine = 44100 / 392;

static void _audio_callback(void* userdata, Uint8* stream, int len) {
    lua_State* L = (lua_State*)userdata;
    Sint16* buffer = (Sint16*)stream;
    int length = len / sizeof(short);
    for(int i = 0; i < length; ++i)
    {
        double time = (double)sample_pos / 44100.0;
        buffer[i] = 64 * sin(2.0f * M_PI * 441.f * time);
        sample_pos++;
    }
}

static BEGIN_FUNCTION(audio, GetCallback)
    PUSH_LUDATA(_audio_callback);
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadOgg)
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, LoadWav)
    CHECK_STRING(path);
    drwav wav;
    Uint32 channels, sample_rate;
    drwav_uint64 frame_count;
    Sint16* frames = drwav_open_file_and_read_pcm_frames_s16(path, &channels, &sample_rate, &frame_count, NULL);
    if (!frames) {
        return luaL_error(L, "Failed to load wav");
    }

    NEW_UDATA(AudioSource, source);
    source->data.data = frames;
    source->data.size = frame_count;
    source->frequency = sample_rate;
    source->channels = channels;
    source->samples = 2048;
END_FUNCTION(1)

static BEGIN_FUNCTION(audio, NewBufferPoll)
    CHECK_INTEGER(size);
    NEW_UDATA(BufferPool, pool);
    pool->pool = malloc(size * sizeof(struct AudioBuffer));
    pool->size = size;
END_FUNCTION(1)

static BEGIN_META_FUNCTION(AudioSource, gc)
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
    END_REG()
    NEW_META(AudioSource);
END_META(1)

BEGIN_MODULE(audio)
    BEGIN_REG(audio)
        REG_FIELD(audio, GetCallback),
        REG_FIELD(audio, LoadOgg),
        REG_FIELD(audio, LoadWav),
    END_REG()
    NEW_MODULE(audio);
    LOAD_META(AudioSource);
END_MODULE(1)


#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"
