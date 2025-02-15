#ifndef SELENE_AUDIO_H_
#define SELENE_AUDIO_H_

#include "selene.h"
#include "lua_helper.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#include "dr_wav.h"
#include "dr_mp3.h"
#include "dr_flac.h"

typedef struct AudioData AudioData;
typedef struct AudioDecoder AudioDecoder;

struct AudioData {
    AudioInfo info;
    int size;
    char* data;
};

struct AudioDecoder {
    AudioInfo info;
    char format;
    union {
        drwav wav;
        drmp3 mp3;
        drflac* flac;
        stb_vorbis* ogg;
    };
    
#if defined(OS_ANDROID)
    void* audio_data;
#endif
};

#if defined(__cplusplus)
extern "C" {
#endif

SELENE_API int s_AudioDecoder_init(lua_State *L, const char *path, int len, AudioDecoder *out);

SELENE_API int s_AudioDecoder_read_s16(AudioDecoder *self, int len, short *data);

SELENE_API int s_AudioDecoder_read_f32(AudioDecoder *self, int len, float *data);
// int s_AudioDecoder_get_chunk(AudioDecoder* self, int len, short* data);
SELENE_API int s_AudioDecoder_seek(AudioDecoder *self, int index);

SELENE_API int s_AudioDecoder_close(AudioDecoder *self);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_AUDIO_H_ */