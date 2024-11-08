#ifndef SELENE_AUDIO_DECODER_H_
#define SELENE_AUDIO_DECODER_H_

#include "selene.h"
#include "common.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#include "dr_wav.h"
#include "dr_mp3.h"
#include "dr_flac.h"

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

SELENE_API int luaopen_audio(lua_State* L);

#endif /* SELENE_AUDIO_DECODER_H_ */
