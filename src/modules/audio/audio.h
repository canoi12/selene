#ifndef AUDIO_H_
#define AUDIO_H_

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#include "dr_wav.h"

enum {
    UNKNOWN_FORMAT = 0,
    WAV_FORMAT,
    OGG_FORMAT,
};

typedef struct Decoder Decoder;
struct Decoder {
    char format;
    union {
        drwav wav;
        stb_vorbis* ogg;
    };
    int sample_rate;
    int channels;
    int bit_depth;
    // Aux to decode buffer chunks
    void* buffer;
    int buffer_len;
};

typedef struct AudioData AudioData;

#endif /* AUDIO_H_ */
