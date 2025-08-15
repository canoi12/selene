#ifndef SELENE_AUDIO_H_
#define SELENE_AUDIO_H_

#include "selene.h"

#if 1 || defined(SELENE_USE_SDL2)
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

#include "dr_wav.h"
#include "dr_mp3.h"
#include "dr_flac.h"
#elif defined(SELENE_USE_MINIAUDIO)
#else
#endif

typedef struct _AudioDevice selene_AudioDevice;

typedef struct _AudioStream selene_AudioStream;
typedef struct _AudioStatic selene_AudioStatic; 

enum AudioFileFormat {
    SELENE_AUDIO_UNKNOWN = 0,
    SELENE_WAV,
    SELENE_OGG,
    SELENE_MP3,
    SELENE_FLAC
};

struct _AudioStatic {
    AudioInfo info;
    int size;
    char* data;
};

struct _AudioStream {
    AudioInfo info;
    enum AudioFileFormat format;
#if !defined(SELENE_USE_MINIAUDIO)
    union {
        drwav wav;
        drmp3 mp3;
        drflac* flac;
        stb_vorbis* ogg;
    };
#endif
#if defined(OS_ANDROID)
    void* audio_data;
#endif
};

struct _AudioDevice {
#if 1 || defined(SELENE_USE_SDL2)
    SDL_AudioDeviceID handle;
#endif
};

#if defined(__cplusplus)
extern "C" {
#endif

SELENE_API int luaopen_audio(lua_State* L);

#if defined(__cplusplus)
}
#endif

#endif /* SELENE_AUDIO_H_ */
