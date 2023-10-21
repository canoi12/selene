#ifndef AUDIO_H_
#define AUDIO_H_

#include "selene.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#include "dr_wav.h"

enum {
    UNKNOWN_FORMAT = 0,
    WAV_FORMAT,
    OGG_FORMAT,
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

typedef struct Decoder Decoder;
struct Decoder {
    char format;
    union {
        drwav wav;
        stb_vorbis* ogg;
    };
    AudioInfo info;
};

typedef struct AudioData AudioData;
struct AudioData {
    Data data;
    AudioInfo info;
};

#endif /* AUDIO_H_ */
