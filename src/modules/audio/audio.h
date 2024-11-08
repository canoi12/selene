#ifndef SELENE_AUDIO_H_
#define SELENE_AUDIO_H_

#include "common.h"
#include "selene.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#include "dr_wav.h"
#include "dr_mp3.h"
#include "dr_flac.h"

#ifndef SELENE_AUDIO_POOL_SIZE
#define SELENE_AUDIO_POOL_SIZE 256
#endif

enum {
    UNKNOWN_FORMAT = 0,
    WAV_FORMAT,
    OGG_FORMAT,
    MP3_FORMAT,
    FLAC_FORMAT,
};

typedef struct AudioInfo AudioInfo;
struct AudioInfo {
    int sample_rate;
    int channels;
    int format;
    int bit_depth;
    int size;
    int frame_count;
};

typedef struct AudioDecoder AudioDecoder;
struct AudioDecoder {
    char format;
    union {
        drwav wav;
        drmp3 mp3;
        drflac* flac;
        stb_vorbis* ogg;
    };
    AudioInfo info;
#if defined(OS_ANDROID)
    void* audio_data;
#endif
};

typedef struct AudioNode AudioNode;
struct AudioNode {
    char playing;
    char loop;
    float volume;
    SDL_AudioStream* stream_ref;
};

typedef struct AudioSystem AudioSystem;
struct AudioSystem {
    int loaded;
    SDL_AudioStream* _stream_pool[SELENE_AUDIO_POOL_SIZE];
};

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Open audio lib and metatables
 */
SELENE_API int luaopen_audio(lua_State* L);

/**
 * Create and returns the metatable of AudioDecoder userdata
 */
SELENE_API int l_AudioDecoder_meta(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_AUDIO_H_ */